#pragma once
#include "PluginProcessor.h"

class GbaButton : public juce::Button
{
public:
    GbaButton(bool roundShape, const juce::String& g) : juce::Button(g), round(roundShape), glyph(g) {}
    void paintButton(juce::Graphics& g, bool, bool down) override
    {
        auto b = getLocalBounds().toFloat().reduced(1.f);
        g.setColour(down ? juce::Colour(0xff9c9db2) : juce::Colour(0xffb9bacb));
        if (round) g.fillEllipse(b); else g.fillRect(b);
        g.setColour(juce::Colour(0x30000000));
        if (round) g.drawEllipse(b, 1.5f); else g.drawRect(b, 1.f);
        g.setColour(juce::Colour(0xff6b6f92));
        g.setFont(juce::Font(b.getHeight() * .5f, juce::Font::bold));
        g.drawText(glyph, getLocalBounds(), juce::Justification::centred);
    }
private:
    bool round; juce::String glyph;
};

class GbaEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GbaEditor(GbaProcessor&);
    ~GbaEditor() override { stopTimer(); }
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(screenRect().toNearestInt()); }
    juce::Rectangle<float> screenRect() const;
    void changeSound(int v, bool absolute);
    void setOct(int d);
    void preview();

    GbaProcessor& proc;
    int oct = 0;
    juce::MidiKeyboardComponent keyboard;
    GbaButton up, left, right, down, btnA, btnB;
};
