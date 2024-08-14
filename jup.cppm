export module jup;

namespace jup {
  // Plays a mono sample at 44100Hz
  export void play(float * samples, unsigned size);

  export constexpr const auto rate = 44100;
} // namespace jup

#ifdef LECO_TARGET_ANDROID
#pragma leco add_impl android
#pragma leco add_library aaudio
#elif LECO_TARGET_APPLE
#pragma leco add_impl apple
#pragma leco add_framework AudioToolbox
#elif LECO_TARGET_WINDOWS
#pragma leco add_impl windows
#pragma leco add_library ole32
#elif LECO_TARGET_LINUX
#pragma leco add_impl linux
#elif LECO_TARGET_WASM
#pragma leco add_impl wasm
#endif
