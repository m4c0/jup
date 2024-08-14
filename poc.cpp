#pragma leco app

import casein;
import jup;
import silog;

extern "C" float sinf(float);

// Stores 1 second of audio. Defined at global scope to avoid taking all stack space in wasm
static float smp[44100] {};

static void play() {
  silog::log(silog::debug, "playing tune");

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
