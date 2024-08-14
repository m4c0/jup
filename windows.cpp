module;
#include <wrl/client.h>
#include <xaudio2.h>

module jup;
import hai;

namespace {
  struct destroyer {
    void operator()(IXAudio2Voice * ptr) { ptr->DestroyVoice(); }
  };

  class stuff {
    Microsoft::WRL::ComPtr<IXAudio2> m_xa2 {};
    hai::holder<IXAudio2MasteringVoice, destroyer> m_main_voice {};
    hai::holder<IXAudio2SourceVoice, destroyer> m_src_voice {};

  public:
    stuff() {
      HRESULT hr {};
      if (FAILED(hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED))) return;
      if (FAILED(hr = XAudio2Create(m_xa2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR))) return;
      if (FAILED(hr = m_xa2->CreateMasteringVoice(&*m_main_voice))) return;

      constexpr const auto channels = 1;
      constexpr const auto rate = 44100;
      constexpr const auto bits_per_sample = 32; // IEEE_FORMAT
      constexpr const auto alignment = (channels * bits_per_sample) / 8;
      WAVEFORMATEX wfx {};
      wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
      wfx.nChannels = channels;
      wfx.nSamplesPerSec = rate;
      wfx.nAvgBytesPerSec = rate * sizeof(float);
      wfx.nBlockAlign = alignment;
      wfx.wBitsPerSample = bits_per_sample;
      wfx.cbSize = 0;

      constexpr const auto ratio = XAUDIO2_DEFAULT_FREQ_RATIO;
      if (FAILED(hr = m_xa2->CreateSourceVoice(&*m_src_voice, &wfx, 0, ratio, nullptr))) return;
    }

    void submit(float * p, unsigned s) const {
      (*m_src_voice)->Stop();
      (*m_src_voice)->FlushSourceBuffers();

      XAUDIO2_BUFFER buf {
        .AudioBytes = s * 4,
        .pAudioData = reinterpret_cast<BYTE *>(p),
      };
      (*m_src_voice)->SubmitSourceBuffer(&buf);
      (*m_src_voice)->Start();
    }
  };
} // namespace

void jup::play(float * samples, unsigned size) {
  static stuff s{};
  s.submit(samples, size);
}
