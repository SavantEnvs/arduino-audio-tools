#include "AudioTools.h"
#include "AudioTools/AudioCodecs/ContainerAVI.h"
using namespace audio_tools;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  NullStream audio, video;
  DemuxerAVI demux;
  demux.setOutputAudio(audio);
  demux.setOutputVideo(video);
  demux.begin();
  size_t off = 0;
  while (off < size) {
    size_t n = demux.write(data + off, size - off);
    if (n == 0) break;
    off += n;
  }
  demux.end();
  return 0;
}
