#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecADTS.h"
using namespace audio_tools;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  NullStream out;
  ADTSDecoder dec;
  dec.setOutput(out);
  dec.begin();
  dec.write(data, size);
  dec.end();
  return 0;
}
