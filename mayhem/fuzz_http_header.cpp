#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/HttpHeader.h"
using namespace audio_tools;

namespace {
// In-memory Client that serves the fuzz input as a received HTTP message.
class MemoryClient : public Client {
 public:
  MemoryClient(const uint8_t *d, size_t n) : data(d), len(n) { setTimeout(0); }
  int connect(IPAddress, uint16_t) override { return 1; }
  int connect(const char *, uint16_t) override { return 1; }
  size_t write(uint8_t) override { return 1; }
  size_t write(const uint8_t *, size_t n) override { return n; }
  int available() override { return (int)(len - pos); }
  int read() override { return pos < len ? data[pos++] : -1; }
  int read(uint8_t *buf, size_t n) override {
    size_t m = n < len - pos ? n : len - pos;
    memcpy(buf, data + pos, m);
    pos += m;
    return (int)m;
  }
  int peek() override { return pos < len ? data[pos] : -1; }
  void flush() override {}
  void stop() override {}
  uint8_t connected() override { return 1; }
  operator bool() override { return true; }

 private:
  const uint8_t *data;
  size_t len;
  size_t pos = 0;
};
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // HttpHeader::read() blocks until the client reports data (HttpHeader.h), so an empty message is not a valid input.
  if (size == 0) return -1;
  {
    MemoryClient in(data, size);
    HttpReplyHeader reply;
    reply.read(in);
    reply.statusCode();
    reply.isChunked();
  }
  {
    MemoryClient in(data, size);
    HttpRequestHeader request;
    request.read(in);
    request.urlPath();
    request.method();
  }
  return 0;
}
