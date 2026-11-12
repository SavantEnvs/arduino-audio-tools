#include "AudioTools.h"
#include "AudioTools/AudioCodecs/ContainerM4A.h"
using namespace audio_tools;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  NullStream out;
  ContainerM4A dec;
  dec.setOutput(out);
  dec.begin();
  dec.write(data, size);
  dec.end();
  return 0;
}
