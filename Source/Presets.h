#pragma once
// w: 0 pulse12.5  1 pulse25  2 pulse50  3 triangle  4 saw  5 sine  6 noise
// ft: 0 lowpass  1 bandpass  2 highpass
struct Preset {
    const char* name;
    int w = 2;
    float a = .005f, d = .2f, s = .6f, r = .15f;
    int o = 0;
    float lp = 9000.f;
    int ft = 0;
    float q = 1.f;
    float det = 0.f, vib = 0.f, sl = 0.f, st = .2f, jp = 0.f;
    int arpN = 0; int arp[4] = {0,0,0,0};
    float fmR = 0.f, fmD = 0.f;
    int ocW = -1; float ocM = 1.f, ocG = 0.f;
    float fe = 0.f; int wob = 0;
};

static constexpr int kNumPresets = 30;
static const Preset kPresets[kNumPresets] = {
 {.name="Pulse Lead",.w=2,.a=.005f,.d=.3f,.s=.6f,.r=.12f},
 {.name="Thin Pulse",.w=0,.d=.25f,.s=.55f,.r=.1f},
 {.name="Classic Quarter",.w=1,.d=.3f,.s=.6f,.r=.1f},
 {.name="Triangle Bass",.w=3,.d=.2f,.s=.8f,.r=.1f,.o=-12},
 {.name="Sub Bass",.w=5,.a=.01f,.d=.2f,.s=.9f,.r=.2f,.o=-24},
 {.name="Saw Lead",.w=4,.a=.01f,.d=.3f,.s=.6f,.r=.15f,.lp=3500.f},
 {.name="Soft Pad",.w=4,.a=.5f,.d=.6f,.s=.7f,.r=.9f,.lp=1400.f,.det=9.f},
 {.name="Chip Arp Major",.w=2,.a=.003f,.d=.2f,.s=.7f,.r=.08f,.arpN=4,.arp={0,4,7,12}},
 {.name="Chip Arp Minor",.w=1,.a=.003f,.d=.2f,.s=.7f,.r=.08f,.arpN=4,.arp={0,3,7,12}},
 {.name="Coin",.w=2,.a=.002f,.d=.25f,.s=0.f,.r=.05f,.jp=1.5f},
 {.name="Laser",.w=1,.a=.002f,.d=.3f,.s=0.f,.r=.05f,.o=12,.sl=.15f,.st=.3f},
 {.name="Power Up",.w=2,.a=.002f,.d=.4f,.s=0.f,.r=.05f,.sl=3.f,.st=.35f},
 {.name="Vibrato Lead",.w=2,.a=.08f,.d=.3f,.s=.7f,.r=.2f,.vib=30.f},
 {.name="Bell FM",.w=5,.a=.002f,.d=1.2f,.s=0.f,.r=.6f,.o=12,.fmR=3.5f,.fmD=1.2f},
 {.name="E-Piano FM",.w=5,.a=.003f,.d=.9f,.s=.15f,.r=.3f,.fmR=1.f,.fmD=.9f},
 {.name="Organ",.w=5,.a=.01f,.d=.1f,.s=.9f,.r=.06f,.ocW=5,.ocM=2.f,.ocG=.6f},
 {.name="Flute",.w=5,.a=.12f,.d=.2f,.s=.8f,.r=.15f,.vib=18.f,.ocW=3,.ocM=2.f,.ocG=.2f},
 {.name="Pluck",.w=1,.a=.002f,.d=.18f,.s=0.f,.r=.05f,.lp=5000.f,.fe=3.f},
 {.name="Kick",.w=5,.a=.001f,.d=.22f,.s=0.f,.r=.03f,.o=-24,.sl=.12f,.st=.12f},
 {.name="Snare",.w=6,.a=.001f,.d=.16f,.s=0.f,.r=.03f,.lp=2500.f,.ft=1,.q=.8f},
 {.name="Hi-Hat",.w=6,.a=.001f,.d=.05f,.s=0.f,.r=.02f,.lp=7500.f,.ft=2},
 {.name="Noise Bass",.w=6,.a=.002f,.d=.2f,.s=.4f,.r=.1f,.o=-12,.lp=500.f},
 {.name="Wind",.w=6,.a=.6f,.d=.5f,.s=.6f,.r=1.f,.lp=900.f,.ft=1,.q=3.f},
 {.name="Super Saw",.w=4,.a=.01f,.d=.3f,.s=.7f,.r=.2f,.lp=4000.f,.det=14.f},
 {.name="Wobble",.w=4,.a=.01f,.d=.2f,.s=.8f,.r=.1f,.o=-12,.lp=700.f,.q=6.f,.wob=1},
 {.name="Brass",.w=4,.a=.04f,.d=.3f,.s=.7f,.r=.12f,.lp=1200.f,.fe=4.f},
 {.name="Harp",.w=3,.a=.002f,.d=.7f,.s=0.f,.r=.3f},
 {.name="Zap",.w=0,.a=.001f,.d=.12f,.s=0.f,.r=.03f,.o=12,.sl=.1f,.st=.1f},
 {.name="Dream Pad",.w=3,.a=.7f,.d=.7f,.s=.8f,.r=1.2f,.det=7.f,.vib=6.f,.ocW=5,.ocM=2.f,.ocG=.3f},
 {.name="8-bit Choir",.w=1,.a=.1f,.d=.3f,.s=.7f,.r=.3f,.det=12.f,.vib=22.f}
};
