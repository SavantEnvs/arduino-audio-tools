# SampleExtractor::write asserts on the result of buffer.write for a stream-controlled sample size

`SampleExtractor::write` (`M4ACommonDemuxer.h`) takes the sample size from the stream (here the `stsz` box's fixed
sample size, 3494969344 bytes), calls `resize(currentSize)` and ignores its result, then does
`assert(buffer.write(data[j]))` on the first `mdat` byte. The resize cannot allocate that size, the buffer rejects the
write, and the assert aborts the process (checked in gdb: `currentSize` 3494969344, buffer length 1024, `j` 0).
Fix: check the result of `resize`/`write`, stop and report the error instead of asserting on stream data.
