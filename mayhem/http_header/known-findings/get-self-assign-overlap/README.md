# HttpHeader::get copies the value onto itself

Calling `reply.get(CONTENT_TYPE)` after `read()` hits `Str::operator=` -> `StrView::set` -> `strncpy` with source and
destination equal (`HttpHeader.h:159`); ASan: strncpy-param-overlap. The harness therefore does not call `get()`.
Fix: skip the assignment when the pointers are equal.
