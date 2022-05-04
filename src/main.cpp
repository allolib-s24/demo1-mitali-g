#include <iostream>
#include <cstdio> // for printing to stdout
#include <vector> // store sample data

// GAMMA
#include "Gamma/Analysis.h"
#include "Gamma/Effects.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"

// ALLOLIB
// for master branch
// #include "al/core.hpp"

// for devel branch
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

// ALLOLIB AUDIO
#include "al/scene/al_PolySynth.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;

struct Sample
{
  int pitch_root = 60;
  int pitch_highest = 127;
  int sample_rate = 44100;
  std::string name;
  std::vector<float> sampleData;

  Sample(std::string filename, int pitch_root, int pitch_highest)
  {
    this->name = filename;
    this->pitch_root = pitch_root;
    this->pitch_highest = pitch_highest;

    // load sample
    SoundFile file;
    file.open(filename.c_str());
    sample_rate = file.sampleRate;
    
    for (long long int i = 0; i < file.frameCount; i++)
    {
      sampleData.push_back(file.getFrame(i)[0]);
    }

    std::cout << "Loaded " << filename << " with " << sampleData.size() << " samples" << std::endl;
  }
};

struct Patch
{
  std::vector<Sample*> samples;
  Patch() {}
  virtual Sample* getSample(int index) { return nullptr; }
};

struct DrumKit : Patch
{
  std::map<std::string, int> sampleIndex;

  DrumKit(std::string directory, std::vector<std::string> filenames, std::map<std::string, int> rootPitches)
  {
    for (int i = 0; i < filenames.size(); i++)
    {
      int pitchAdjust = 0;

      if (rootPitches.count(filenames[i]))
      {
        pitchAdjust = rootPitches[filenames[i]];
      }

      samples.push_back(
        new Sample(
          "timbre/" + directory + "/" + filenames[i] + ".wav",
          0 - pitchAdjust,
          0
        )
      );

      sampleIndex[filenames[i]] = i;
    }
  }

  Sample* getSample(int index)
  {
    return samples[index];
  }

  int s(std::string sampleName) {
    return sampleIndex[sampleName];
  }
};

struct Timbre : Patch
{
  Timbre(std::string name, std::vector<int> pitches, int correct=0)
  {
    for (int i = 0; i < pitches.size(); i++)
    {
      int pitch = pitches[i];
      int nextPitch;

      if (i < pitches.size() - 1)
      {
        nextPitch = pitches[i + 1];
      }
      else
      {
        nextPitch = 127;
      }

      samples.push_back(
        new Sample(
          "timbre/" + name + "/" + std::to_string(pitch) + ".wav",
          pitch - correct,
          nextPitch - 1 - correct
        )
      );
    }
  }

  Sample* getSample(int pitch)
  {
    // Find the sample that's most optimal for the pitch
    for (int i = 0; i < samples.size(); i++)
    {
      if (pitch <= samples[i]->pitch_highest)
      {
        return samples[i];
      }
    }

    return samples[0]; // Fallback to single sample
  }
};

DrumKit drumKit(
  "Drum",
  std::vector<std::string> {
    "KICK-POP",
    "KICK-DAC",
    "KICK-SK",
    
    "SNARE-DAC",
    "CLAP-POP",

    "HAT-DANCE",
    "HAT-DAC",
    "HAT-SK",

    "SLAP-POP",
    "SLIDE-POP",
    "SLIDE-POP-OFFSET",
    
    "CRASH-LO",
    "CRASH-HI",
    "CRASH-HI",

    "SFX-BELL",
    "SFX-BOWL-LO",
    "SFX-BOWL-HI",
    "SFX-BREATH",
    "SFX-DROP-LO",
    "SFX-DROP-HI",
    "SFX-GOURD",
    "SFX-LOTUSBEND",
    "SFX-MK939",
    "SFX-SHAKE-OFFSET",
    "SFX-SHAKE"
  },
  std::map<std::string, int> {
    {"CRASH-LO", 3},
    {"SFX-LOTUSBEND", -2},
    {"SFX-MK939", -1}
  }
);

std::vector<Patch*> SoundBank = {
  /* 00 */ new Timbre("LofiPCM/BUTTERFLY", {84, 88}, -2),
  /* 01 */ new Timbre("LofiPCM/CROSSEDKEYMATRIX", {84}, -2),
  /* 02 */ new Timbre("LofiPCM/DANCINGDELICATESTRING", {68}, -2),
  /* 03 */ new Timbre("LofiPCM/NIGHTMARKET", {49, 61, 64, 68, 71, 75}, -2),
  /* 04 */ new Timbre("LofiPCM/SKPIZZ", {36, 48, 60, 72}, -2),
  /* 05 */ new Timbre("LofiPCM/TADPOLE", {36, 45, 48, 60, 72, 84, 96}, -2),

  /* 06 */ new Timbre("LofiSynth/CORRODE", {65, 70}),
  /* 07 */ new Timbre("LofiSynth/DOORBELL", {65, 82, 86}),
  /* 08 */ new Timbre("LofiSynth/ENCHANTED", {60, 77, 78, 79, 80}),
  /* 09 */ new Timbre("LofiSynth/HIVEHOLE", {51, 63, 66, 70, 73}),
  /* 10 */ new Timbre("LofiSynth/LOWBATTERY", {85}),
  /* 11 */ new Timbre("LofiSynth/MYSTICAL", {87, 94}),
  /* 12 */ new Timbre("LofiSynth/PATCH34", {58, 61, 65, 68, 75, 80, 87}),
  /* 13 */ new Timbre("LofiSynth/SWEETDREAMS", {70, 85}),
  /* 14 */ new Timbre("LofiSynth/UNSETTLINGBASS", {39, 42, 44, 48, 49, 51, 56, 57}),
  /* 15 */ new Timbre("LofiSynth/WIRE", {75, 82, 85, 92}),

  /* 16 */ new Timbre("PopSynth/CHORD-CHORUS", {51, 63, 66, 70, 73}),
  /* 17 */ new Timbre("PopSynth/CHORD-INTRO", {51, 63, 66, 70, 73}),
  /* 18 */ new Timbre("PopSynth/CHORD-WIRE", {51, 63, 66, 70, 73}),
  /* 19 */ new Timbre("PopSynth/HOUSEPIANO", {51, 63, 66, 70, 73}),
  /* 20 */ new Timbre("PopSynth/LOGBASS", {32, 39, 42, 44, 48, 49, 56, 58}),
  /* 21 */ &drumKit
};

class PCMEnv : public SynthVoice
{
public:
  gam::Pan<> mPan;
  gam::Sine<> mOsc;
  gam::Env<3> mAmpEnv;

  float attenuation = 0;
  float rate = 0;
  float position = 0;
  Sample* sampleRef = nullptr;;
  Patch* currentPatch = nullptr;
  int sampleLength = 0;

  void init() override
  {
    // Intialize envelope
    mAmpEnv.curve(0); // make segments lines
    mAmpEnv.levels(0, 1, 1, 0);
    mAmpEnv.sustainPoint(2);

    // Set up parameters
    createInternalTriggerParameter("timbre", 0, 0, SoundBank.size() - 1);
    createInternalTriggerParameter("amplitude", 1, 0.0, 20.0);
    createInternalTriggerParameter("midiNote", 1, 0, 127);
    createInternalTriggerParameter("attackTime", 0.001, 0.001, 3.0);
    createInternalTriggerParameter("releaseTime", 0.1, 0.001, 10.0);
    createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
    createInternalTriggerParameter("interpolate", 0, 0, 1);
  }

  float linear_interpolate(std::vector<float>& data, float position, int length) {
    int floored_position = floor(position);
    float current_item = data[floored_position];
    int next_position = floored_position + 1;

    if (next_position < length)
    {
      float next_item = data[next_position];
      float fraction = position - floored_position;

      return current_item + fraction * (next_item - current_item);
    }

    return current_item;
  }

  void onProcess(AudioIOData &io) override
  {
    bool interpolate = getInternalParameterValue("interpolate");
    mAmpEnv.lengths()[0] = getInternalParameterValue("attackTime");
    mAmpEnv.lengths()[2] = getInternalParameterValue("releaseTime");
    mPan.pos(getInternalParameterValue("pan"));
    
    while (io())
    {
      float s1 = 0;
      float s2;

      // Patch
      if (position >= sampleLength) {
        rate = 0;
      }

      if (rate > 0) {
        if (interpolate) {
          s1 = linear_interpolate(sampleRef->sampleData, position, sampleLength);
        } else {
          s1 = sampleRef->sampleData.at(position);
        }
          
        s1 = s1 / attenuation;
        s1 = s1 * mAmpEnv() * getInternalParameterValue("amplitude");
      }

      position += rate;

      // Pan
      mPan(s1, s1, s2);

      // Output
      io.out(0) = s1;
      io.out(1) = s2;

      if (position >= sampleLength || mAmpEnv.done()) {
        free();
      }
    }
  }

  void set(int timbre, float midiNote, float amplitude, bool interpolate, float releaseTime)
  {
    setInternalParameterValue("timbre", timbre);
    setInternalParameterValue("midiNote", midiNote);
    setInternalParameterValue("amplitude", amplitude);
    setInternalParameterValue("interpolate", interpolate);
    setInternalParameterValue("releaseTime", releaseTime);
  }

  void onTriggerOn() override {
    float midiNote = getInternalParameterValue("midiNote");

    // Update currentPatch with timbre parameter
    this->currentPatch = SoundBank[getInternalParameterValue("timbre")];

    // Use midiNote to find the best sample in timbre
    this->sampleRef = this->currentPatch->getSample(
      static_cast<int>(floor(midiNote))
    );

    std::cout << midiNote << "\tSample:" << sampleRef->name << std::endl;

    if (getInternalParameterValue("timbre") == 21)
    {
      midiNote = 0; // Don't use index for pitch on drum kit
    }

    // Convert midiNote to frequency to set playback rate
    this->rate = pow(2.f, (midiNote - this->sampleRef->pitch_root) / 12.f);
    
    // Prepare playback of sample
    this->sampleLength = this->sampleRef->sampleData.size();
    this->attenuation = 2;
    this->position = 0;

    // Reset envelope
    mAmpEnv.reset();
  }

  void onTriggerOff() override {
    mAmpEnv.release();
  }
};





// We make an app.
class MyApp : public App
{
public:
  int octaveShift = 0;

  // GUI manager for SineEnv voices
  // The name provided determines the name of the directory
  // where the presets and sequences are stored
  SynthGUIManager<PCMEnv> synthManager{"PCMEnv"};

  // This function is called right after the window is created
  // It provides a grphics context to initialize ParameterGUI
  // It's also a good place to put things that should
  // happen once at startup.
  void onCreate() override
  {
    navControl().active(false); // Disable navigation via keyboard
    gam::sampleRate(audioIO().framesPerSecond()); // Set Gamma sample rate
    imguiInit();
    synthManager.synthRecorder().verbose(true);
    sequencer().playSequence();
  }

  // The audio callback function. Called when audio hardware requires data
  void onSound(AudioIOData &io) override
  {
    synthManager.render(io); // Render audio
  }

  void onAnimate(double dt) override
  {
    imguiBeginFrame();
    synthManager.drawSynthControlPanel();
    imguiEndFrame();
  }

  // The graphics callback function.
  void onDraw(Graphics &g) override
  {
    g.clear();
    synthManager.render(g);
    imguiDraw();
  }

  // Whenever a key is pressed, this function is called
  bool onKeyDown(Keyboard const &k) override
  {
    if (ParameterGUI::usingKeyboard())
    { // Ignore keys if GUI is using
      // keyboard
      return true;
    }
    if (k.shift())
    {
      // If shift pressed then keyboard sets preset
      int presetNumber = asciiToIndex(k.key());
      synthManager.recallPreset(presetNumber);
    }
    else
    {
      // If - or = pressed change octave
      if (k.key() == '-')
      {
        octaveShift--;
      }
      else if (k.key() == '=')
      {
        octaveShift++;
      }
      else
      {
        // Otherwise trigger note for polyphonic synth
        int midiNote = asciiToMIDI(k.key(), octaveShift * 12);
        if (midiNote > 0)
        {
          synthManager.voice()->setInternalParameterValue(
              "midiNote", midiNote);
          synthManager.triggerOn(midiNote);
        }
      }
    }
    return true;
  }

  // Whenever a key is released this function is called
  bool onKeyUp(Keyboard const &k) override
  {
    int midiNote = asciiToMIDI(k.key(), octaveShift * 12);
    if (midiNote > 0)
    {
      synthManager.triggerOff(midiNote);
    }
    return true;
  }

  void onExit() override { imguiShutdown(); }

  SynthSequencer &sequencer() { return synthManager.synthSequencer(); }
};



































































SynthSequencer* seq;
const float TRIPLET = (1.f/12.f)*8.f;
int track = 0;
int timbre = 0;
int transpose = 0;
float release = 0;
float volume = 1;
float strength = 1;
float tune = 0;
bool interpolate = false;
std::vector<float> cursors = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Insert cursor position for each track

void n(int note, float length=1, float gap=0)
{
  seq->add<PCMEnv>(cursors.at(track), length / 8).set(
    timbre,
    note + 12 + transpose + tune,
    (volume * strength) * 1.375,
    interpolate,
    release
  );

  cursors[track] += (length + gap) / 8;
}

void d(std::string sample, float gap=0, float length=64)
{
  seq->add<PCMEnv>(cursors.at(track), length / 8).set(
    21,
    drumKit.s(sample),
    (volume * strength) * 1.375,
    true,
    0.001
  );

  cursors[track] += gap / 8;
}

void c(int note, float length=16, float gap=0)
{
  n(note, length, gap - length);
}

void r(float length)
{
  cursors[track] += length / 8;
}







void reset()
{
  interpolate = true;
  volume = 1;
  transpose = 0;
  tune = 0;
  release = 0.001;
}

void pt_intro_pianolo()
{
  timbre = 5;
  interpolate = false;
  release = 1;
  strength = 1;
  tune = -12;

  n(33, 6);
  n(40, 10, 10);
  n(26, 12);
  n(28, 9);
  n(29, 6, -1);
  n(36, 3);
  n(45, 8);
  n(28, 8, 9 + (16 * 3));
  std::cout << track << cursors[track] << std::endl;
}

void pt_intro_pianohi()
{
  timbre = 0;
  interpolate = false;
  release = 20;
  strength = 3;
  tune = -12;
  r(3);

  n(81, 5);

  n(91, TRIPLET);
  n(86, TRIPLET);
  n(88, TRIPLET);
  n(84, TRIPLET);
  n(86, TRIPLET);
  n(81, TRIPLET);
  n(84, TRIPLET);
  n(79, TRIPLET);
  n(81, TRIPLET);
  n(74, TRIPLET);
  n(76, TRIPLET, TRIPLET * 2);

  n(87, TRIPLET, TRIPLET * 4);
  r(4 * 3);
  r(16 * 2);
  r(7);
  n(76, 1, 8);
  r(16 * 3);
  std::cout << track << cursors[track] << std::endl;
}

void pt_intro_pizz()
{
  timbre = 4;
  interpolate = false;
  release = 50;
  strength = 1;
  tune = -12;
  r(6);

  n(57);
  n(64, 0.5);
  n(67, 0.5);
  n(69, 1, 7 + 7.5);
  n(67, 0.5, 2);
  n(73, 0.5, 1);
  n(62, 0.5, 0.5);
  n(74, 0.5, 0.5);
  n(79, 0.5, 0.5);
  n(59, 0.5, 2);
  n(72, 1, 3);
  n(84, 1, 10 + 12);
  n(56, 1, 3);
  r(64);
}

void pt_intro_str()
{
  timbre = 2;
  interpolate = false;
  release = 0.625;
  strength = 0.75;
  tune = -12 + 15.f/100.f;
  r(6);

  n(95, 1, 4);
  n(57, 9, 6);

  // Chords
  c(53, 10);
  c(57, 10);
  c(60, 10);
  c(64, 10);
  n(50, 10);

  c(55, 11);
  c(57, 11);
  c(59, 11);
  c(62, 11);
  c(64, 11);
  n(52, 11);

  c(53, 11);
  c(57, 11);
  c(60, 11);
  c(62, 11);
  c(64, 11);
  c(67, 11);
  r(2);

  n(72, 3, 1);
  n(83, 4, -2);
  n(72, 8, -3);

  c(67, 17);
  r(2);
  c(55, 13);
  c(59, 13);
  c(64, 13);
  c(69, 13);
  n(52, 13, 5);

  r(16 * 3);
}

void pt_intro_pianomid()
{
  timbre = 1;
  interpolate = true;
  release = 20;
  strength = 1;
  tune = -7.f/100.f;
  r(14);

  c(69, 16);
  r(5);
  n(71, 11, -4);
  n(79, 4);
  r(8);
  n(88, 1, 5);
  n(93, 1, 11);
  n(81, 3);
  n(76, 4);
  n(79, 4);
  n(74, 13);
  r(8);
  n(69, 8);

  r(16 * 2);
  std::cout << track << cursors[track] << std::endl;
}

void pt_intro_pianolegato()
{
  timbre = 5;
  interpolate = false;
  release = 20;
  strength = 1;
  tune = -12.2f;
  r(27);

  n(74, 4, -2);
  n(69, 4, -2);
  n(76, 4, -2);
  n(79, 4, -2);
  n(84, 4, -2);
  n(79, 4, 1);
  n(81, 4, 2);

  c(60, 4);
  r(1.5);
  c(62, 4);
  r(0.5);
  c(64, 4);
  r(0.5);
  c(69, 4);
  r(0.5);
  c(76, 4);
  r(0.5);
  c(79, 4);
  r(1);
  n(84, 4.5, 2);

  n(76, 4, -1);
  n(79, 4, -1);
  n(84, 4, -2);
  n(72, 4, -1);
  c(55, 30);
  r(1);
  n(69, 4, 2);
  n(52, 3 + 16);

  r(16 * 2);
}

void pt_intro_fx_doorbell()
{
  timbre = 7;
  interpolate = true;
  release = 20;
  strength = 4;
  tune = 0;
  r(11);

  n(64, 3, 2);
  r(5);
  n(85, 1, 2);
  n(81, 2, 6);
  r(16 * 6);
}

void pt_intro_fx_arp()
{
  timbre = 15;
  interpolate = true;
  release = 10;
  strength = 10;
  tune = -12;

  r(16);
  n(81);
  n(84);
  n(79);
  n(81);
  n(76);
  n(79);
  n(74);
  n(76);
  n(72);
  n(74);
  n(69);
  n(72);
  n(74);
  n(79);
  n(81);
  n(84);
  n(91, 16, 8);
  n(81, 3);
  n(76, 2);

  strength = 15;
  n(79, 3);

  r(16 * 4);
}

void pt_intro_fx_enchant()
{
  timbre = 8;
  interpolate = true;
  release = 0.15;
  tune = -24;
  r(7);

  strength = 15;
  n(71, 2, 9);
  strength = 6;
  n(91, 7);
  n(90, 3);
  n(89, 3);
  n(88, 7, 10);
  r(16 * 3);

  r(11);
  n(64, 5 + 12);
  
  r(4);
}

void pt_intro_fx_lead()
{
  timbre = 11;
  interpolate = true;
  release = 10;
  strength = 5;
  tune = -12;

  r(20);
  n(79, 13, -7);
  n(81, 13, -7);
  n(86, 13, -5);
  n(91, 13);
  n(91, 13, -2);

  r(16 + 4);

  n(79, 13, -7);
  n(81, 13, -7);
  n(86, 13, -5);
  n(91, 13);
  n(93, 7, 4);
}

void pt_intro_fx_corrode()
{
  timbre = 6;
  interpolate = true;
  release = 50;
  strength = 10;
  tune = -12;

  r(56);
  n(69, 3);
  n(64, 3);
  n(67, 4);
  n(62, 10, 4);
  r(16 * 3);
}













void pt_bass(bool funky_ending=false, bool funky_intro=false)
{
  if (funky_intro)
  {
    n(28, 1, 1);
    n(28, 2, 1);
    n(28, 1, 1);
    n(28, 1, 3);
    n(28, 1, 1);
  }
  else
  {
    n(26, 1, 1);
    n(26, 2, 1);
    n(26, 1, 1);
    n(26, 1, 3);
    n(26, 1, 1);
  }
  
  n(31);
  n(35);
  n(38);

  n(29, 1, 1);
  n(29, 2, 1);
  n(29, 1, 1);
  n(29, 1, 2);
  n(29);
  n(19);
  n(29);
  n(24, 1, 1);
  n(30);

  n(31, 1, 1);
  n(31, 2, 2);
  n(31);
  n(31, 1, 2);
  n(31);
  n(36, 2);
  n(40);
  n(43);
  n(45);

  if (!funky_ending)
  {
    n(26, 2);
    n(26, 2, 1);
    n(26, 1, 1);
    n(26, 1, 2);
    n(33, 1, 1);
  }
  else
  {
    n(26, 2);
    n(26, 2);
    n(38);
    n(26);
    n(50);
    n(44);
    n(48);
    n(49);
    n(49, 0.5, -0.5);
    n(50, 2);
  }

  // bass slide
  n(41, 0.5);
  n(43, 0.5);
  n(44, 0.5, -0.5);
  n(37, 0.5);
  n(31, 0.5);
  n(25, 0.5);
  n(21, 0.5);
  n(20, 0.5);
  n(17, 0.5);
}

void pt_bass_simple()
{
  n(26, 4, 1);
  n(26, 1, 1);
  n(26, 5);
  n(36, 2);
  n(38, 2);

  n(29, 4, 1);
  n(29, 1, 1);
  n(29, 5);
  n(36, 2);
  n(38, 2);

  n(33, 4, 1);
  n(33, 1, 1);
  n(33, 5);
  n(45, 2);
  n(38, 2);

  n(26, 4, 1);
  n(26, 1, 1);
  n(26, 2, 1);
  n(26, 2);
  n(36);
  n(43);
  n(38);
  n(29);
}

void pt_chord_progression()
{
  c(50);
  c(53);
  c(57);
  c(60);
  c(64);
  n(38, 16);

  c(55);
  c(57);
  c(60);
  c(64);
  n(41, 16);

  c(50);
  c(55);
  c(57);
  c(59);
  c(64);
  n(43, 16);

  c(52);
  c(53);
  c(55);
  c(57);
  c(60);
  n(38, 16);
}

void pt_pluck_chord(int measure, int gap=0)
{
  switch (measure)
  {
  default:
    c(50, 3);
    c(53, 3);
    c(57, 3);
    c(60, 3);
    c(64, 3);
    n(38, 3, gap);
    break;
  case 1:
    c(55, 3);
    c(57, 3);
    c(60, 3);
    c(64, 3);
    n(41, 3, gap);
    break;
  case 2:
    c(50, 3);
    c(55, 3);
    c(57, 3);
    c(59, 3);
    c(64, 3);
    n(43, 3, gap);
    break;
  case 3:
    c(52, 3);
    c(53, 3);
    c(55, 3);
    c(57, 3);
    c(60, 3);
    n(38, 3, gap);
  }
}

void pt_pluck_chords()
{
  pt_pluck_chord(0);
  pt_pluck_chord(0, 1);
  pt_pluck_chord(0);
  pt_pluck_chord(0, 3);

  pt_pluck_chord(1);
  pt_pluck_chord(1, 1);
  pt_pluck_chord(1);
  pt_pluck_chord(1, 3);

  pt_pluck_chord(2);
  pt_pluck_chord(2, 1);
  pt_pluck_chord(2);
  pt_pluck_chord(2, 3);

  pt_pluck_chord(3);
  pt_pluck_chord(3, 1);
  pt_pluck_chord(3);
  pt_pluck_chord(3, 3);
}

void pt_drum_intro()
{
  strength = 2.25;
  ////////////////////////////////
  r(16 * 2);
  r(1.5);

  d("SFX-MK939", 0.5);

  r(5.5);

  d("SFX-LOTUSBEND", 6.5, 6.5);
  d("KICK-DAC", 0.5);
  d("KICK-DAC", 0.5);
  d("KICK-DAC");
  d("CRASH-LO", 1);
  
  r(16 * 4);
  r(12);

  d("KICK-SK");
  d("SNARE-DAC", 2);
  d("KICK-SK", 2);
}

void pt_drum_chorus()
{
  strength = 2.25;
  ////////////////////////////////

  d("CRASH-LO");
  d("KICK-POP", 4);
  //
  d("KICK-POP");
  d("CLAP-POP", 4);
  //
  d("KICK-POP", 4);
  //
  d("KICK-POP");
  d("CLAP-POP");
  d("SLIDE-POP-OFFSET");
  d("SLAP-POP", 2);

  d("SLIDE-POP");
  d("CLAP-POP", 1);

  d("KICK-POP", 1);

  ////////////////

  d("KICK-POP", 4);
  //
  d("KICK-POP");
  d("CLAP-POP", 4);
  //
  d("KICK-POP", 3);
  //
  d("CLAP-POP");
  d("SLAP-POP", 0.5);
  d("SLAP-POP", 0.5);

  d("KICK-POP");
  d("CLAP-POP");
  d("SLAP-POP", 1);
  d("CLAP-POP", 1);
  d("CLAP-POP");
  d("SLAP-POP", 1);
  d("CLAP-POP");
  d("KICK-POP", 1);

  ////////////////

  d("KICK-POP", 4);
  //
  d("KICK-POP");
  d("CLAP-POP", 4);
  //
  d("KICK-POP", 4);
  //
  d("KICK-POP");
  d("CLAP-POP");
  d("SLAP-POP", 3);

  d("KICK-POP", 1);

  ////////////////

  d("KICK-POP", 4);
  //
  d("KICK-POP");
  d("CLAP-POP", 3);

  d("CLAP-POP", 1);
  //
  d("SLAP-POP");
  d("KICK-POP", 1);
  d("CLAP-POP", 1);
  d("SLIDE-POP-OFFSET", 1);
  d("KICK-POP", 1);
  //
  d("CLAP-POP");
  d("SLAP-POP");
  d("SLIDE-POP");
  d("KICK-POP", 2);
  d("KICK-POP", 2);
}




void pt_beat_drop()
{
  track = 0;
  timbre = 14;
  interpolate = true;
  volume = 0.825;
  strength = 5;
  transpose = 0;
  tune = 0;
  release = 0.001;
  pt_bass();

  track = 1;
  timbre = 20;
  volume = 1.25;
  pt_bass();

  track = 2;
  timbre = 17;
  volume = 0.75;
  pt_chord_progression();

  track = 3;
  timbre = 3;
  volume = 1.125;
  pt_pluck_chords();

  track = 4;
  timbre = 0;
  volume = 0.35;
  transpose = -12;
  tune = 0.5f;
  release = 3;
  r(16*2);
  r(9);
  n(79);
  n(81);
  n(84);
  n(86);
  n(88);
  n(91);
  n(93, 8, -5);
  n(81, 8, -3);
  n(86, 5);
  r(4);

  track = 11;
  volume = 1;
  pt_drum_chorus();
}


int main()
{
  // Create app instance
  MyApp app;

  // Set up audio
  app.configureAudio(48000., 128, 2, 0);

  // Create sequence
  seq = &app.sequencer();

  ////////////////////////////////
  // INTRO
  ////////////////////////////////
  volume = 0.575;
  track = 0;
  pt_intro_pianolo();
  track = 1;
  pt_intro_pianohi();
  track = 2;
  pt_intro_pianomid();
  track = 3;
  pt_intro_pianolegato();
  track = 4;
  pt_intro_pizz();
  track = 5;
  pt_intro_str();
  track = 6;
  pt_intro_fx_doorbell();
  track = 7;
  pt_intro_fx_arp();
  track = 8;
  pt_intro_fx_enchant();
  track = 9;
  pt_intro_fx_lead();
  track = 10;
  pt_intro_fx_corrode();
  track = 11;
  volume = 1;
  pt_drum_intro();


  ////////////////////////////////
  // BEAT DROP
  ////////////////////////////////
  pt_beat_drop();


  ////////////////////////////////
  // VERSE 1, PART 1
  ////////////////////////////////
  strength = 5;
  reset();
  track = 0;
  timbre = 14;
  volume = 0.5;
  pt_bass(false, true);

  reset();
  track = 1;
  timbre = 15;
  transpose = -12;
  release = 25;
  r(14);

  n(79);
  c(69, 11);
  c(72, 11);
  c(74, 11);
  c(83, 11);
  n(67, 11);
  
  r(6);

  c(65, 12);
  c(69, 12);
  c(72, 12);
  c(76, 12);
  n(62, 12);

  r(4);

  c(62, 13);
  c(67, 13);
  c(69, 13);
  c(72, 13);
  c(76, 13);
  n(60, 13);

  r(3);

  reset();
  track = 2;
  timbre = 21;
  volume = 0.625;
  ////////////////
  d("KICK-POP");
  r(4);
  d("CLAP-POP");
  d("KICK-POP");
  r(4);
  d("KICK-POP");
  r(2);
  d("SLIDE-POP-OFFSET");
  r(2);
  d("CLAP-POP");
  d("SLAP-POP");
  d("KICK-POP");
  r(2);
  d("SLIDE-POP");
  d("CLAP-POP");
  r(1);
  d("KICK-POP");
  r(1);
  ////////////////
  for (int i = 0; i < 2; i++)
  {
    d("KICK-POP");
    r(4);
    d("CLAP-POP");
    d("KICK-POP");
    r(4);
    d("KICK-POP");
    r(4);
    d("CLAP-POP");
    d("SLAP-POP");
    d("KICK-POP");
    r(3);
    d("KICK-POP");
    r(1);
  }
  ////////////////
  d("KICK-POP");
  r(4);
  d("CLAP-POP");
  d("KICK-POP");
  r(4);
  d("KICK-POP");
  d("SLIDE-POP-OFFSET");
  r(4);
  d("CLAP-POP");
  d("SLIDE-POP");
  d("KICK-POP");
  r(3);
  d("KICK-POP");
  r(1);

  track = 3;
  r(64);
  track = 4;
  r(64);
  track = 5;
  r(64);


  ////////////////////////////////
  // VERSE 1, PART 2
  ////////////////////////////////
  reset();
  track = 0;
  timbre = 14;
  volume = 0.5;
  pt_bass(false, true);

  reset();
  track = 1;
  timbre = 15;
  transpose = -12;
  release = 25;
  c(64, 15);
  c(72, 15);
  c(79, 15);
  c(81, 15);
  n(91, 4);
  n(84, 4);
  n(93, 4);
  n(98, 4, -1);

  c(69, 11);
  c(72, 11);
  c(74, 11);
  c(83, 11);
  n(67, 11);

  r(6);

  c(65, 12);
  c(69, 12);
  c(72, 12);
  c(76, 12);
  n(62, 12);

  r(4);

  c(62, 13);
  c(67, 13);
  c(69, 13);
  c(72, 13);
  c(76, 13);
  n(60, 13);

  r(3);

  reset();
  track = 2;
  timbre = 18;
  transpose = -12;
  r(14);

  n(79);
  c(69, 11);
  c(72, 11);
  c(74, 11);
  c(83, 11);
  n(67, 11);
  
  r(6);

  c(65, 12);
  c(69, 12);
  c(72, 12);
  c(76, 12);
  n(62, 12);

  r(4);

  c(62, 13);
  c(67, 13);
  c(69, 13);
  c(72, 13);
  c(76, 13);
  n(60, 13);

  r(3);

  reset();
  track = 3;
  timbre = 21;
  volume = 0.625;
  ////////////////
  d("KICK-POP");
  r(4);
  d("CLAP-POP");
  d("KICK-POP");
  r(4);
  d("KICK-POP");
  r(2);
  d("SLIDE-POP-OFFSET");
  r(2);
  d("CLAP-POP");
  d("SLAP-POP");
  d("KICK-POP");
  r(2);
  d("SLIDE-POP");
  d("CLAP-POP");
  r(1);
  d("KICK-POP");
  r(1);
  ////////////////
  d("KICK-POP");
  r(4);
  d("CLAP-POP");
  d("KICK-POP");
  r(2);
  d("SLIDE-POP-OFFSET");
  r(2);
  d("KICK-POP", 1);
  d("SFX-GOURD", 1);
  d("KICK-POP");
  d("SFX-BELL");
  d("SLIDE-POP");
  r(1);
  d("SFX-BOWL-LO");
  d("SFX-DROP-HI");
  r(1);
  d("CLAP-POP");
  d("SLAP-POP");
  d("KICK-POP");
  r(3);
  d("KICK-POP");
  r(1);
  ////////////////
  d("KICK-POP");
  r(4);
  d("CLAP-POP");
  d("KICK-POP");
  r(4);
  d("KICK-POP");
  r(4);
  d("CLAP-POP");
  d("SLAP-POP");
  d("KICK-POP");
  r(3);
  d("KICK-POP");
  r(1);
  ////////////////
  d("KICK-POP");
  r(4);
  d("CLAP-POP");
  d("KICK-POP");
  r(4);
  d("KICK-POP");
  d("SLIDE-POP-OFFSET");
  r(4);
  d("CLAP-POP");
  d("SLIDE-POP");
  d("KICK-POP");
  r(2);
  d("KICK-POP");
  r(1);
  d("SFX-BOWL-HI");
  r(1);

  reset();
  track = 4;
  timbre = 5;
  interpolate = false;
  release = 20;
  volume = 0.125;
  tune = -11.625f;
  r(4);
  n(79, 7, -3);
  n(81, 7, -3);
  n(86, 7, -3);
  r(16);
  r(8);
  release = 0.01;
  volume = 0.0625;
  n(43, 7, 1);
  r(16);

  track = 5;
  r(64);

  // Start app to play sequence
  app.start();
  return 0;
}
