// Known-answer probe: decodes fixed inputs through the real library and prints the parsed values.
// mayhem/test.sh compares this output against exact expected strings, from bash.
#include <stdio.h>
#include <string.h>
#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecWAV.h"
#include "AudioTools/AudioCodecs/CodecADTS.h"
using namespace audio_tools;

namespace {
class CapturePrint : public Print {
 public:
  size_t write(uint8_t b) override { buf[n++ % sizeof(buf)] = b; return 1; }
  size_t write(const uint8_t *d, size_t len) override {
    for (size_t i = 0; i < len; i++) write(d[i]);
    return len;
  }
  uint8_t buf[256];
  size_t n = 0;
};

void hex(const char *label, const uint8_t *d, size_t n) {
  printf("%s=", label);
  for (size_t i = 0; i < n; i++) printf("%02x", d[i]);
  printf("\n");
}
}  // namespace

int main() {
  // RIFF/WAVE, PCM, 2 channels, 22050 Hz, 16 bit, 8 bytes of payload
  static const uint8_t wav[] = {
      'R', 'I', 'F', 'F', 0x2c, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
      16, 0, 0, 0, 1, 0, 2, 0, 0x22, 0x56, 0, 0, 0x88, 0x58, 0x01, 0,
      4, 0, 16, 0, 'd', 'a', 't', 'a', 8, 0, 0, 0,
      0x01, 0x02, 0x03, 0x04, 0xa0, 0xb0, 0xc0, 0xd0};
  CapturePrint out;
  WAVDecoder dec;
  dec.setOutput(out);
  dec.begin();
  dec.write(wav, sizeof(wav));
  dec.end();
  WAVAudioInfo &info = dec.audioInfoEx();
  printf("wav.channels=%d\n", (int)info.channels);
  printf("wav.sample_rate=%d\n", (int)info.sample_rate);
  printf("wav.bits=%d\n", (int)info.bits_per_sample);
  printf("wav.data_length=%u\n", (unsigned)info.data_length);
  printf("wav.out_bytes=%u\n", (unsigned)out.n);
  hex("wav.payload", out.buf, out.n < 8 ? out.n : 8);

  // ADTS header: AAC-LC, 44100 Hz, 2 channels, frame length 20
  uint8_t adts[] = {0xff, 0xf1, 0x50, 0x80, 0x02, 0x80, 0xfc};
  ADTSParser parser;
  parser.begin();
  bool ok = parser.parse(adts);
  printf("adts.valid=%d\n", (int)ok);
  printf("adts.sample_rate=%d\n", parser.getSampleRate());
  printf("adts.channels=%d\n", (int)parser.data().channel_cfg);
  printf("adts.profile=%d\n", (int)parser.data().profile);
  printf("adts.frame_length=%u\n", (unsigned)parser.getFrameLength());
  return 0;
}
