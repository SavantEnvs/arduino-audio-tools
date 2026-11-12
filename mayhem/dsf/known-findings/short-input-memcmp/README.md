# DSFDecoder::write reads 4 bytes from a shorter buffer

`DSFDecoder::processHeader` (`src/AudioTools/AudioCodecs/CodecDSF.h`) runs `memcmp(data, "DSD ", 4)` before
checking `len >= 4`; any write of 0-3 bytes (including 0) over-reads the caller's buffer.
ASan: heap-buffer-overflow in `memcmp` <- `DSFDecoder::processHeader`.
Fix: return 0 when `len < 4` (and bound `findTag`/`parseFMT`/`parseData` by `len`).

The reproducer is a 1-byte write and does **not** fire through `mayhem/fuzz_dsf.cpp`: the harness passes the exact-size
input (no padding), but returns -1 for inputs below 4 bytes, because `processHeader` unconditionally reads the 4 magic
bytes. Without that, libFuzzer's startup inputs (empty, then 1 byte) halt the run before any other input executes, so
the target cannot iterate. To see the finding, call `DSFDecoder::write` on an exact-size heap buffer of 0-3 bytes.
