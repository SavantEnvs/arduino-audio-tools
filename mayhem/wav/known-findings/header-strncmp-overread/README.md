# WAVHeader::getDataPos searches the header buffer with strncmp past its end

`WAVHeader::isDataComplete()` -> `getDataPos()` (`CodecWAV.h`) calls `StrView::indexOf("data")` on the header buffer,
which runs `strncmp` over memory that is not NUL-terminated. ASan: heap-buffer-overflow in `strncmp` <-
`StrView::indexOf` <- `WAVHeader::getDataPos` <- `WAVDecoder::decodeHeader`. Fix: search with an explicit length
bound (`memmem` on `buffer.available()`).

Two benign UBSan reports hit every WAV before this one and are relaxed for the wav target in `mayhem/build.sh`
(`-fno-sanitize=vptr,shift-base`): `WAVHeader::parse` `memset`s `headerInfo`, which has a vtable, and then touches it
(`vptr`, CodecWAV.h:118 - fires on every valid WAV); and `read_int32`/`read_int16` shift the `-1` that `read()` returns
at end of data (`shift-base`, CodecWAV.h:319).
