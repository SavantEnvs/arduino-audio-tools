#include "AudioTools.h"
#include "AudioTools/AudioCodecs/MP3Parser.h"
using namespace audio_tools;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  NullStream out;
  MP3ParserEncoder enc;
  enc.setOutput(out);
  enc.begin();
  enc.write(data, size);
  enc.end();
  return 0;
}
