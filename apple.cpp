module;
#include <AudioToolbox/AudioComponent.h>
#include <AudioToolbox/AudioOutputUnit.h>
#include <AudioToolbox/AudioUnitProperties.h>

#if TARGET_OS_OSX
#define COMPONENT_SUBTYPE kAudioUnitSubType_DefaultOutput;
#else
#define COMPONENT_SUBTYPE kAudioUnitSubType_RemoteIO;
#endif

module jup;
import hai;
import silog;
import traits;

struct deleter {
  void operator()(AudioComponentInstance i) {
    AudioOutputUnitStop(i);
    AudioUnitUninitialize(i);
    AudioComponentInstanceDispose(i);
  }
};

static hai::array<float> g_buffer {};
static int g_frame;
static OSStatus render(void * /*ref*/, AudioUnitRenderActionFlags * /*flags*/, const AudioTimeStamp * /*timestamp*/,
                       UInt32 /*bus_number*/, UInt32 number_frames, AudioBufferList * data) {
  auto * f_data = static_cast<float *>(data->mBuffers[0].mData);
  for (auto i = 0; i < number_frames; i++, f_data++) {
    *f_data = (g_frame >= g_buffer.size()) ? 0 : g_buffer[g_frame++];
  }
  return noErr;
}

static AudioComponentInstance err(const char * msg) {
  silog::log(silog::error, "%s", msg);
  return nullptr;
}
static AudioComponentInstance create_ci() {
  AudioComponentDescription acd {};
  acd.componentType = kAudioUnitType_Output;
  acd.componentSubType = COMPONENT_SUBTYPE;
  acd.componentManufacturer = kAudioUnitManufacturer_Apple;
  acd.componentFlags = 0;
  acd.componentFlagsMask = 0;

  AudioComponent ac = AudioComponentFindNext(nullptr, &acd);
  if (ac == nullptr) return err("Could not find audio component");

  AudioComponentInstance tone_unit {};
  if (AudioComponentInstanceNew(ac, &tone_unit) != noErr) return err("Could not create audio component instance");

  AURenderCallbackStruct rcs {};
  rcs.inputProc = render;
  if (AudioUnitSetProperty(tone_unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &rcs, sizeof(rcs))
      != noErr)
    return err("Could not setup audio callback");

  AudioStreamBasicDescription sbd {};
  sbd.mSampleRate = 44100;
  sbd.mFormatID = kAudioFormatLinearPCM;
  sbd.mFormatFlags = static_cast<unsigned>(kAudioFormatFlagsNativeFloatPacked) | kAudioFormatFlagIsNonInterleaved;
  sbd.mBytesPerPacket = sizeof(float);
  sbd.mFramesPerPacket = 1;
  sbd.mBytesPerFrame = sizeof(float) / 1;
  sbd.mChannelsPerFrame = 1;
  sbd.mBitsPerChannel = sizeof(float) * 8;
  if (AudioUnitSetProperty(tone_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &sbd, sizeof(sbd))
      != noErr)
    return err("Could not setup audio stream format");

  if (AudioUnitInitialize(tone_unit) != noErr) return err("could not initialise audio unit");

  if (AudioOutputUnitStart(tone_unit) != noErr) return err("could not start audio unit");

  silog::log(silog::info, "AudioToolbox initialised");
  return tone_unit;
}

void jup::play(float * samples, unsigned size) {
  static hai::value_holder<AudioComponentInstance, deleter> ci { create_ci() };
  if (!*ci) return;

  hai::array<float> next { size };
  for (auto i = 0; i < size; i++) next[i] = samples[i];
  g_buffer = traits::move(next);
  g_frame = 0;
}
