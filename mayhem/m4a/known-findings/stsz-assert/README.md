# M4ACommonDemuxer::onStsz asserts on a malformed stsz box

`M4ACommonDemuxer::onStsz` (`M4ACommonDemuxer.h`) `assert()`s on box contents taken from the stream, so a crafted
`stsz` box aborts the process. Fix: validate the box size and return instead of asserting.

## Cross-input state in the m4a target

`SampleExtractor::currentSampleSize()` keeps `static size_t last_index` / `last_size` as function-local statics, shared
by every decoder instance in the process, so a fresh decoder can see the previous input's cached sample size. The
harness cannot reset them without patching upstream. The reproducers in this directory fire from a fresh process
(`fuzz_m4a-standalone`), so they do not depend on it.
