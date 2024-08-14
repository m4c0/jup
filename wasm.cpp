module jup;

extern "C" __attribute__((import_module("jup"), import_name("play"))) void jup_play(float * samples, unsigned size);

void jup::play(float * samples, unsigned size) { jup_play(samples, size); }
