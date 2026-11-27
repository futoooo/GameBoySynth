#include "PluginEditor.h"
#include "Presets.h"

static juce::String G(const char* utf8) { return juce::String(juce::CharPointer_UTF8(utf8)); }
static const juce::Colour kLcd(0xffc9e6a0), kScreen(0xff252b2a), kScopeBg(0xff1b201f);

GbaEditor::GbaEditor(GbaProcessor& p)
    : AudioProcessorEditor(&p), proc(p),
      keyboard(p.keys, juce::MidiKeyboardComponent::horizontalKeyboard),
      up(false, G("\xe2\x96\xb2")), left(false, G("\xe2\x97\x80")),
      right(false, G("\xe2\x96\xb6")), down(false, G("\xe2\x96\xbc")),
      btnA(true, "A"), btnB(true, "B")
{
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xffd8dcc8));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xff1c2120));
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, kScreen);
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour(0xaa9bdc6f));
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour(0x449bdc6f));
    keyboard.setScrollButtonsVisible(false);
    keyboard.setAvailableRange(12, 120);
    keyboard.setLowestVisibleKey(60);
    for (auto* c : std::initializer_list<juce::Component*>{ &keyboard, &up, &left, &right, &down, &btnA, &btnB })
        addAndMakeVisible(c);

    left.onClick  = [this] { changeSound(-1, false); };
    right.onClick = [this] { changeSound(1, false); };
    up.onClick    = [this] { setOct(1); };
    down.onClick  = [this] { setOct(-1); };
    btnA.onClick  = [this] { changeSound(juce::Random::getSystemRandom().nextInt(kNumPresets), true); };
    btnB.onClick  = [this] { changeSound(0, true); };

    setResizeLimits(520, 312, 1400, 840);
    getConstrainer()->setFixedAspectRatio(5.0 / 3.0);
    setSize(780, 468);
    startTimerHz(30);
}

juce::Rectangle<float> GbaEditor::screenRect() const
{
    const float W = (float)getWidth(), H = (float)getHeight();
    return { .305f * W, .25f * H, .39f * W, .41f * H };
}

void GbaEditor::resized()
{
    const float W = (float)getWidth(), H = (float)getHeight();
    auto R = [](float x, float y, float w, float h) { return juce::Rectangle<float>(x, y, w, h).toNearestInt(); };
    auto sc = screenRect();
    keyboard.setBounds(R(sc.getX() + sc.getWidth() * .03f, sc.getY() + sc.getHeight() * .66f,
                         sc.getWidth() * .94f, sc.getHeight() * .30f));
    keyboard.setKeyWidth(keyboard.getWidth() / 10.0f);

    const float s = .118f * W, a = s / 3.f, cx = .172f * W, cy = .41f * H;
    up.setBounds   (R(cx - a / 2, cy - s / 2, a, a));
    left.setBounds (R(cx - s / 2, cy - a / 2, a, a));
    right.setBounds(R(cx + s / 2 - a, cy - a / 2, a, a));
    down.setBounds (R(cx - a / 2, cy + s / 2 - a, a, a));
    const float r = .029f * W;
    btnA.setBounds(R(.865f * W - r, .40f * H - r, 2 * r, 2 * r));
    btnB.setBounds(R(.785f * W - r, .447f * H - r, 2 * r, 2 * r));
}

void GbaEditor::paint(juce::Graphics& g)
{
    const float W = (float)getWidth(), H = (float)getHeight();
    // translucent "glacier" shell, colours taken from the photo
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffd0d3e3), 0, 0, juce::Colour(0xffa2a7c4), 0, H, false));
    g.fillRoundedRectangle(0, 0, W, H, W * .07f);
    g.setColour(juce::Colours::white.withAlpha(.35f));
    g.drawRoundedRectangle(1, 1, W - 2, H - 2, W * .07f, 2.f);

    g.setColour(juce::Colour(0xff17181f));
    g.fillRoundedRectangle(.26f * W, .17f * H, .475f * W, .65f * H, W * .025f);
    g.setColour(juce::Colour(0xffcfd2e0));
    g.setFont(juce::Font(H * .033f, juce::Font::bold));
    g.drawText("GAME BOY ADVANCE", juce::Rectangle<float>(.26f * W, .735f * H, .475f * W, .06f * H), juce::Justification::centred);

    auto sc = screenRect();
    g.setColour(kScreen); g.fillRect(sc);

    const int idx = proc.sound->get();
    g.setColour(kLcd);
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), sc.getHeight() * .075f, juce::Font::bold));
    auto hd = sc.withHeight(sc.getHeight() * .12f).reduced(sc.getWidth() * .03f, 0);
    g.drawText(juce::String(idx + 1).paddedLeft('0', 2) + "/30  " + juce::String(kPresets[idx].name).toUpperCase(),
               hd, juce::Justification::centredLeft);
    g.drawText("OKT " + juce::String(oct > 0 ? "+" : "") + juce::String(oct), hd, juce::Justification::centredRight);

    auto sp = juce::Rectangle<float>(sc.getX() + sc.getWidth() * .03f, sc.getY() + sc.getHeight() * .13f,
                                     sc.getWidth() * .94f, sc.getHeight() * .50f);
    g.setColour(kScopeBg); g.fillRect(sp);
    g.setColour(juce::Colour(0xff2c3633));
    g.drawHorizontalLine((int)sp.getCentreY(), sp.getX(), sp.getRight());
    for (int i = 1; i < 8; ++i) g.drawVerticalLine((int)(sp.getX() + sp.getWidth() * i / 8), sp.getY(), sp.getBottom());

    std::array<float, 1024> snap;
    const int w = proc.wpos.load();
    for (int i = 0; i < 1024; ++i)
        snap[(size_t)i] = proc.scope[(size_t)((w + GbaProcessor::scopeSize - 1024 + i) % GbaProcessor::scopeSize)].load();
    int st = 0;
    for (int i = 1; i < 512; ++i) if (snap[(size_t)i - 1] < 0 && snap[(size_t)i] >= 0) { st = i; break; }
    juce::Path path;
    const int px = (int)sp.getWidth();
    for (int x = 0; x < px; ++x) {
        const float v = juce::jlimit(-1.f, 1.f, snap[(size_t)(st + x * 511 / juce::jmax(1, px))] * 3.f);
        const float y = sp.getCentreY() - v * sp.getHeight() * .45f;
        x == 0 ? path.startNewSubPath(sp.getX(), y) : path.lineTo(sp.getX() + (float)x, y);
    }
    g.setColour(kLcd);
    g.strokePath(path, juce::PathStrokeType(2.f));

    // start / select / speaker decoration
    g.setColour(juce::Colour(0xffb9bacb));
    g.fillEllipse(.228f * W - W * .014f, .613f * H - W * .014f, W * .028f, W * .028f);
    g.fillEllipse(.228f * W - W * .014f, .69f * H - W * .014f, W * .028f, W * .028f);
    g.setColour(juce::Colour(0x99505682));
    for (int i = 0; i < 6; ++i)
        g.drawLine(.76f * W, (.575f + i * .018f) * H, .85f * W, (.555f + i * .018f) * H, 2.f);
}

void GbaEditor::changeSound(int v, bool absolute)
{
    const int n = absolute ? v : (proc.sound->get() + v + kNumPresets) % kNumPresets;
    proc.sound->beginChangeGesture();
    *proc.sound = n;
    proc.sound->endChangeGesture();
    preview();
}

void GbaEditor::setOct(int d)
{
    oct = juce::jlimit(-2, 2, oct + d);
    keyboard.setLowestVisibleKey(60 + 12 * oct);
    repaint();
}

void GbaEditor::preview()
{
    const int n = 60 + 12 * oct;
    auto* pr = &proc;
    pr->keys.noteOn(1, n, .8f);
    juce::Timer::callAfterDelay(250, [pr, n] { pr->keys.noteOff(1, n, 0.f); });
}
