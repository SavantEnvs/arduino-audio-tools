#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecDSF.h"
using namespace audio_tools;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // DSFDecoder::processHeader() unconditionally reads the 4 magic bytes (CodecDSF.h:187), so a shorter write is outside
  // its contract; the 1-byte input libFuzzer tries at startup would otherwise halt the run (known-findings/short-input-memcmp).
  if (size < 4) return -1;
  NullStream out;
  DSFDecoder dec;
  dec.setOutput(out);
  dec.begin();
  dec.write(data, size);
  dec.end();
  return 0;
}
