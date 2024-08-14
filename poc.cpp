#pragma leco app

import casein;
import silog;

extern "C" float sinf(float);

namespace jup {
  // Plays a mono sample at 44100Hz
  void play(float * samples, int size) {}
} // namespace jup

static void play() {
  silog::log(silog::debug, "playing tune");

  float smp[44100] {}; // 1 second of audio
  for (auto i = 0; i < 44100; i++) {
    smp[i] = 0.25f * sinf(i);
  }
  jup::play(smp, 44100);
}

static struct init {
  init() {
    using namespace casein;

    handle(MOUSE_DOWN, play);
    handle(KEY_DOWN, play);
  }
} i;
