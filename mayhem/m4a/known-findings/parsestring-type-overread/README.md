# MP4Parser::parseString reads the next box type without checking the remaining length

`MP4Parser::parseString(str, len, ...)` (`MP4Parser.h`) calls `isValidType(str + idx + 4)` on every iteration without
comparing `idx + 8` with `len`, so the four `isalnum()` reads run past the end of the data. It is reached from
`M4ACommonDemuxer::onMp4a` (`parseString(box.data + 28, box.data_size - 28, ...)`, where `data_size < 28` makes `len`
negative) and from `M4ACommonDemuxer::onStsd`. ASan: heap-buffer-overflow (or SEGV for a huge offset) in
`isValidType` <- `parseString`. Reproducers: `reproducer-onmp4a`, `reproducer-onstsd`. Fix: stop when `idx + 8 > len`
(and reject `len < 0`) before reading the type.
