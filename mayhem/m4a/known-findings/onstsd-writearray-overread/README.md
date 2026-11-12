# M4ACommonDemuxer::onStsd copies box.data_size bytes although only box.available are present

`onStsd` (`M4ACommonDemuxer.h`) calls `buffer.writeArray(box.data, box.data_size)`, but while a box is delivered in
chunks only `box.available` bytes of `box.data` are valid, so `BaseBuffer::writeArray` reads past the parse buffer.
ASan: heap-buffer-overflow in `BaseBuffer<unsigned char>::writeArray` <- `SingleBuffer::writeArray` <- `onStsd`.
Fix: write `box.available` bytes, as `onStsz` does.
