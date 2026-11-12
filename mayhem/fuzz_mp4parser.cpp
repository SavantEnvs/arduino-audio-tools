#include "AudioTools.h"
#include "AudioTools/AudioCodecs/MP4Parser.h"
using namespace audio_tools;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  MP4Parser parser;
  parser.setCallback([](MP4Parser::Box &, void *) {});
  parser.begin();
  size_t off = 0;
  while (off < size) {
    size_t n = parser.write(data + off, size - off);
    if (n == 0) break;
    off += n;
  }
  return 0;
}
