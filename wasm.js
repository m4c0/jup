!function() {
  var ctx = null;
  var src = null;

  function play(ptr, qty) {
    if (!ctx) ctx = new AudioContext();
    if (src) src.stop();

    const smps = new Float32Array(leco_exports.memory.buffer, ptr, qty);

    const buf = ctx.createBuffer(1, qty, 44100);
    buf.getChannelData(0).set(smps);

    src = ctx.createBufferSource();
    src.connect(ctx.destination);
    src.buffer = buf;
    src.start();
  }

  leco_imports.jup = { play };
}();
