#!/usr/bin/env bash
#
# mayhem/build.sh — build the arduino-audio-tools fuzz harnesses (one libFuzzer binary + one standalone
# reproducer each) and the known-answer probe that mayhem/test.sh asserts on.
#
# arduino-audio-tools is a header-only C++ library: the "project" is compiled into each harness TU, so the
# library code carries $SANITIZER_FLAGS, SanCov (-fsanitize=fuzzer-no-link) and DWARF 3 like the harness.
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"

# Arduino-Emulator (pinned in mayhem/Dockerfile): static lib + the Arduino core headers the library includes.
EMU=/opt/toolchains/deps/arduino-emulator-350f8da
INCLUDES="-I$SRC/src -I$EMU/src/ArduinoCore-API -I$EMU/src/ArduinoCore-Linux/cores/arduino \
-I$EMU/src/ArduinoCore-Linux/libraries -I$EMU/src/ArduinoCore-API/api/deprecated"
# ARDUINO/IS_DESKTOP select the desktop code paths of the library; -w silences upstream's header warnings.
CXXFLAGS_COMMON="-std=c++17 -DARDUINO -DIS_DESKTOP -w $INCLUDES"
LIBS="$EMU/lib/libarduino_emulator.a -lpthread -lm"

TARGETS="mts wav adts dsf m4a mp3parser mp4parser avi mpg http_header"

OBJ=/tmp/mayhem-obj
mkdir -p "$OBJ"

# Leak detection off at build time (rules/c.md): ASan + UBSan stay fully active.
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o "$OBJ/lsan_off.o"
# libFuzzer-less run-once driver for the -standalone reproducers (compiled as C to keep its C linkage).
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OBJ/standalone_main.o"

# wav: WAVHeader::parse() memset()s a polymorphic object (UBSan vptr, every valid WAV) and the readers left-shift the
# -1 returned at end of data (shift-base). Both are benign in practice and would stop every run before the parser is
# reached; only these two checks are relaxed, for this target only (see mayhem/wav/known-findings).
relax_for() { case "$1" in wav) echo "-fno-sanitize=vptr,shift-base" ;; esac; }
export -f relax_for

build_target() {
  local t=$1 relax
  relax="$(relax_for "$t")"
  $CXX $SANITIZER_FLAGS $relax $DEBUG_FLAGS -fsanitize=fuzzer-no-link $CXXFLAGS_COMMON $LIB_FUZZING_ENGINE \
      "$SRC/mayhem/fuzz_$t.cpp" "$OBJ/lsan_off.o" $LIBS -o "/mayhem/fuzz_$t"
  $CXX $SANITIZER_FLAGS $relax $DEBUG_FLAGS $CXXFLAGS_COMMON \
      "$SRC/mayhem/fuzz_$t.cpp" "$OBJ/standalone_main.o" "$OBJ/lsan_off.o" $LIBS -o "/mayhem/fuzz_$t-standalone"
}
export -f build_target
export OBJ SRC CXXFLAGS_COMMON LIBS

printf '%s\n' $TARGETS | xargs -P "$MAYHEM_JOBS" -I{} bash -c 'build_target {}'

# Test/oracle build: the project's NORMAL flags (no sanitizers, no DWARF pin), dynamically linked so the
# sabotage shim can neuter it.
$CXX -O1 $COVERAGE_FLAGS $CXXFLAGS_COMMON "$SRC/mayhem/kat/kat_probe.cpp" $LIBS -o /mayhem/kat_probe
readelf -l /mayhem/kat_probe | grep -q INTERP || { echo "kat_probe must be dynamically linked" >&2; exit 1; }
