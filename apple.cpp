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

struct deleter {
  void operator()(AudioComponentInstance i) {
    AudioOutputUnitStop(i);
    AudioUnitUninitialize(i);
    AudioComponentInstanceDispose(i);
  }
};

static OSStatus render(void * /*ref*/, AudioUnitRenderActionFlags * /*flags*/, const AudioTimeStamp * /*timestamp*/,
                       UInt32 /*bus_number*/, UInt32 number_frames, AudioBufferList * data) {
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

  silog::log(silog::info, "AudioToolbox initialised");
  return tone_unit;
}

void jup::play(float * samples, unsigned size) {
  static hai::value_holder<AudioComponentInstance, deleter> ci { create_ci() };
}
