# ESDSParser::parse receives a wrapped size from a box whose size field is below 8

`MP4Parser::parseString` computes `readU32(str + idx) - 8` into a `size_t` without checking that the box size is at
least 8, so an `esds` box with size field 0 gets a huge `data_size`. `ESDSParser::parse(data, size)` then evaluates
`data + size` (`M4ACommonDemuxer.h:58`), which UBSan reports as a pointer-overflow (reported by Mayhem as an
out-of-bounds read). Fix: reject box sizes below the 8-byte header in `parseString`.
