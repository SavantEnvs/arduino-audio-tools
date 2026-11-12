#!/usr/bin/env bash
#
# mayhem/test.sh — known-answer test: run mayhem/kat/kat_probe.cpp (built by build.sh with the library's
# normal flags) and compare what the library parsed out of fixed WAV and ADTS inputs against values worked
# out by hand from those bytes. The comparison runs here in bash, so a neutered probe can't pass it.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SRC:=/mayhem}"
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

PROBE=/mayhem/kat_probe
[ -x "$PROBE" ] || { echo "missing $PROBE (mayhem/build.sh should have built it)" >&2; emit_ctrf "arduino-audio-tools-kat" 0 1 0; exit 1; }

out="$("$PROBE" 2>/dev/null)"
passed=0; failed=0
check() {
  local key="$1" want="$2" got
  got="$(printf '%s\n' "$out" | sed -n "s/^${key}=//p")"
  if [ "$got" = "$want" ]; then
    passed=$((passed + 1))
  else
    echo "FAIL $key: want '$want' got '$got'" >&2
    failed=$((failed + 1))
  fi
}

# WAV: RIFF/WAVE PCM, 2 channels, 22050 Hz, 16 bit, 8 payload bytes
check wav.channels 2
check wav.sample_rate 22050
check wav.bits 16
check wav.data_length 8
check wav.out_bytes 8
check wav.payload 01020304a0b0c0d0
# ADTS: AAC-LC (profile 1), 44100 Hz (index 4), 2 channels, frame length 20
check adts.valid 1
check adts.sample_rate 44100
check adts.channels 2
check adts.profile 1
check adts.frame_length 20

emit_ctrf "arduino-audio-tools-kat" "$passed" "$failed" 0
