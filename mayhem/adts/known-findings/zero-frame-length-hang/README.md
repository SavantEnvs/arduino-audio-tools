# ADTSDecoder::end() loops forever on a header with frame_length 0

`ADTSDecoder::parseBuffer` (`CodecADTS.h`) accepts a header whose frame length is smaller than the 7-byte header
(here 0), then `buffer.clearArray(frameLength)` removes nothing and the `while (true)` loop spins on the same bytes.
Fix: treat `frame_length < 7` as an invalid header.
