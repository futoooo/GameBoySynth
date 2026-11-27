#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

namespace {
struct GbaSound : juce::SynthesiserSound {
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class GbaVoice : public juce::SynthesiserVoice
{
public:
    explicit GbaVoice(juce::AudioParameterInt* s) : soundParam(s) {}
    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<GbaSound*>(s) != nullptr; }

    void startNote(int midi, float vel, juce::SynthesiserSound*, int) override
    {
        p = &kPresets[juce::jlimit(0, kNumPresets - 1, soundParam->get())];
        baseF = 440.0 * std::pow(2.0, (midi + p->o - 69) / 12.0);
        t = 0; for (auto& x : ph) x = 0;
        modPh = 0; low = band = 0; nz = 0; nph = 0;
        level = (p->w == 6 ? 0.5f : 0.26f) * (0.5f + 0.5f * vel);
        adsr.setSampleRate(getSampleRate());
        adsr.setParameters({ p->a, p->d, p->s, p->r });
        adsr.noteOn();
    }
    void stopNote(float, bool tail) override
    {
        if (tail) adsr.noteOff();
        else { adsr.reset(); clearCurrentNote(); p = nullptr; }
    }
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (p == nullptr || getCurrentlyPlayingNote() < 0) return;
        const double sr = getSampleRate();
        const double TP = juce::MathConstants<double>::twoPi;

        auto osc = [&](int k, double fr, int w) -> double {
            if (w == 6) {
                nph += juce::jlimit(0.2, 4.0, fr / 220.0);
                if (nph >= 1.0) { nph -= 1.0; nz = rng.nextFloat() * 2.f - 1.f; }
                return nz;
            }
            ph[k] += fr / sr; if (ph[k] >= 1.0) ph[k] -= 1.0;
            const double x = ph[k];
            switch (w) {
                case 0: return x < .125 ? 1.0 : -1.0;
                case 1: return x < .25 ? 1.0 : -1.0;
                case 2: return x < .5 ? 1.0 : -1.0;
                case 3: return 4.0 * std::abs(x - .5) - 1.0;
                case 4: return 2.0 * x - 1.0;
                default: return std::sin(TP * x);
            }
        };

        for (int i = 0; i < num; ++i)
        {
            if (!adsr.isActive()) { clearCurrentNote(); p = nullptr; break; }

            double f = baseF;
            if (p->arpN)    f *= std::pow(2.0, p->arp[(int)(t / 0.07) % p->arpN] / 12.0);
            if (p->sl > 0)  f *= std::pow((double)p->sl, juce::jmin(t / p->st, 1.0));
            if (p->jp > 0 && t >= 0.07) f *= p->jp;
            if (p->vib > 0) f *= std::pow(2.0, p->vib * std::sin(TP * 5.5 * t) / 1200.0);

            double x;
            if (p->fmR > 0) {
                modPh += f * p->fmR / sr; if (modPh >= 1.0) modPh -= 1.0;
                const double idx = (p->fmD / p->fmR) * (0.1 + 0.9 * std::exp(-t / (p->d / 3.0)));
                ph[0] += f / sr; if (ph[0] >= 1.0) ph[0] -= 1.0;
                x = std::sin(TP * (ph[0] + idx * std::sin(TP * modPh)));
            } else x = osc(0, f, p->w);

            if (p->det > 0) {
                x += .8 * osc(1, f * std::pow(2.0, p->det / 1200.0), p->w);
                x += .8 * osc(2, f * std::pow(2.0, -p->det / 1200.0), p->w);
                x *= .45;
            }
            if (p->ocW >= 0) x += p->ocG * osc(3, f * p->ocM, p->ocW);

            double fc = p->lp;
            if (p->fe > 1) fc *= 1.0 + (p->fe - 1.0) * std::exp(-t / .15);
            if (p->wob)    fc += p->lp * .8 * std::sin(TP * 5.0 * t);
            fc = juce::jlimit(40.0, sr * .17, fc);
            const double fq = 2.0 * std::sin(juce::MathConstants<double>::pi * fc / sr);
            const double qq = 1.0 / juce::jmax(.5f, p->q);
            low += fq * band;
            const double high = x - low - qq * band;
            band += fq * high;
            const double y = p->ft == 0 ? low : (p->ft == 1 ? band * qq * 2.0 : high);

            const float s = (float)(y * adsr.getNextSample() * level);
            for (int c = 0; c < out.getNumChannels(); ++c) out.addSample(c, start + i, s);
            t += 1.0 / sr;
        }
    }

private:
    juce::AudioParameterInt* soundParam;
    const Preset* p = nullptr;
    juce::ADSR adsr;
    juce::Random rng;
    double baseF = 440, t = 0, ph[4] = {0,0,0,0}, modPh = 0, low = 0, band = 0, nph = 0;
    float nz = 0, level = .26f;
};
}

GbaProcessor::GbaProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    addParameter(sound  = new juce::AudioParameterInt({ "sound", 1 }, "Sound", 0, kNumPresets - 1, 0));
    addParameter(volume = new juce::AudioParameterFloat({ "volume", 1 }, "Volume", 0.f, 1.f, .7f));
    for (int i = 0; i < 8; ++i) synth.addVoice(new GbaVoice(sound));
    synth.addSound(new GbaSound());
}

void GbaProcessor::prepareToPlay(double sr, int) { synth.setCurrentPlaybackSampleRate(sr); }

void GbaProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    buffer.clear();
    keys.processNextMidiBuffer(midi, 0, n, true);
    synth.renderNextBlock(buffer, midi, 0, n);
    buffer.applyGain(volume->get());

    const float* l = buffer.getReadPointer(0);
    int w = wpos.load();
    for (int i = 0; i < n; ++i) { scope[(size_t)w].store(l[i]); w = (w + 1) % scopeSize; }
    wpos.store(w);
}

juce::AudioProcessorEditor* GbaProcessor::createEditor() { return new GbaEditor(*this); }

void GbaProcessor::getStateInformation(juce::MemoryBlock& d)
{
    juce::MemoryOutputStream s(d, false);
    s.writeInt(sound->get()); s.writeFloat(volume->get());
}
void GbaProcessor::setStateInformation(const void* data, int size)
{
    if (size < 8) return;
    juce::MemoryInputStream s(data, (size_t)size, false);
    *sound = s.readInt(); *volume = s.readFloat();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GbaProcessor(); }
