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
  std::vector<float> sampleData;

  Sample(std::string filename, int pitch_root, int pitch_highest)
  {
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

struct Timbre
{
  std::vector<Sample*> samples;

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

std::vector<Timbre> SoundBank = {
  Timbre("LofiPCM/BUTTERFLY", {84, 88}, -2),
  Timbre("LofiPCM/CROSSEDKEYMATRIX", {84}, -2),
  Timbre("LofiPCM/DANCINGDELICATESTRING", {68}, -2),
  Timbre("LofiPCM/NIGHTMARKET", {49, 61, 64, 68, 71, 75}, -2),
  Timbre("LofiPCM/SKPIZZ", {36, 48, 60, 72}, -2),
  Timbre("LofiPCM/TADPOLE", {36, 45, 48, 60, 72, 84, 96}, -2),

  Timbre("LofiSynth/CORRODE", {65, 70}),
  Timbre("LofiSynth/DOORBELL", {65, 82, 86}),
  Timbre("LofiSynth/ENCHANTED", {53, 60, 77, 78, 79, 80}),
  Timbre("LofiSynth/HIVEHOLE", {51, 63, 66, 70, 73}),
  Timbre("LofiSynth/LOWBATTERY", {85}),
  Timbre("LofiSynth/MYSTICAL", {87, 94}),
  Timbre("LofiSynth/PATCH34", {58, 61, 65, 68, 75, 80, 87}),
  Timbre("LofiSynth/SWEETDREAMS", {70, 85}),
  Timbre("LofiSynth/UNSETTLINGBASS", {39, 42, 44, 48, 49, 51, 56, 57}),
  Timbre("LofiSynth/WIRE", {75, 82, 85, 92}),

  Timbre("PopSynth/CHORD-CHORUS", {51, 63, 66, 70, 73}),
  Timbre("PopSynth/CHORD-INTRO", {51, 63, 66, 70, 73}),
  Timbre("PopSynth/CHORD-WIRE", {51, 63, 66, 70, 73}),
  Timbre("PopSynth/HOUSEPIANO", {51, 63, 66, 70, 73}),
  Timbre("PopSynth/LOGBASS", {32, 39, 42, 44, 48, 49, 56, 58}),
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
  Timbre* currentTimbre = nullptr;
  int sampleLength = 0;

  void init() override
  {
    // Intialize envelope
    mAmpEnv.curve(0); // make segments lines
    mAmpEnv.levels(0, 1, 1, 0);
    mAmpEnv.sustainPoint(2);

    // Set up parameters
    createInternalTriggerParameter("timbre", 0, 0, SoundBank.size() - 1);
    createInternalTriggerParameter("amplitude", 1, 0.0, 1.0);
    createInternalTriggerParameter("midiNote", 1, 0, 127);
    createInternalTriggerParameter("attackTime", 0, 0.001, 3.0);
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

      // Instrument
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

  void set(int timbre, int midiNote, float amplitude)
  {
    setInternalParameterValue("timbre", timbre);
    setInternalParameterValue("midiNote", midiNote);
    setInternalParameterValue("amplitude", amplitude);
  }

  void onTriggerOn() override {
    int midiNote = getInternalParameterValue("midiNote");

    // Update currentTimbre with timbre parameter
    this->currentTimbre = &SoundBank[getInternalParameterValue("timbre")];

    // Use midiNote to find the best sample in timbre
    this->sampleRef = this->currentTimbre->getSample(midiNote);
    std::cout << "Sample: " << sampleRef->pitch_root << std::endl;

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
    navControl().active(false); // Disable navigation via keyboard, since we
                                // will be using keyboard for note triggering

    // Set sampling rate for Gamma objects from app's audio
    gam::sampleRate(audioIO().framesPerSecond());

    imguiInit();

    // Play example sequence. Comment this line to start from scratch
    // synthManager.synthSequencer().playSequence("synth1.synthSequence");
    synthManager.synthRecorder().verbose(true);
  }

  // The audio callback function. Called when audio hardware requires data
  void onSound(AudioIOData &io) override
  {
    synthManager.render(io); // Render audio
  }

  void onAnimate(double dt) override
  {
    // The GUI is prepared here
    imguiBeginFrame();
    // Draw a window that contains the synth control panel
    synthManager.drawSynthControlPanel();
    imguiEndFrame();
  }

  // The graphics callback function.
  void onDraw(Graphics &g) override
  {
    g.clear();
    // Render the synth's graphics
    synthManager.render(g);

    // GUI is drawn here
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
};

int main()
{
  // Create app instance
  MyApp app;

  // Set up audio
  app.configureAudio(48000., 128, 2, 0);

  app.start();

  return 0;
}




// struct MyApp : App {
//   Mesh mesh;
//   double phase = 0;

//   void onCreate() override {
//     addTetrahedron(mesh);
//     lens().near(0.1).far(25).fovy(45);
//     nav().pos(0, 0, 4);
//     nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
//   }

//   void onAnimate(double dt) override {
//     double period = 10;
//     phase += dt / period;
//     if (phase >= 1.) phase -= 1.;
//   }

//   void onDraw(Graphics& g) override {
//     g.clear(0, 0, 0);
//     g.polygonLine();
//     g.pushMatrix();
//     g.rotate(phase * 360, 0, 1, 0);
//     g.color(1);
//     g.draw(mesh);
//     g.popMatrix();
//   }
// };

// int main() {
//   MyApp app;
//   app.dimensions(600, 400);
//   app.start();
// }
