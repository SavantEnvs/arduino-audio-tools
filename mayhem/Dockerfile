# syntax=docker/dockerfile:1.7
FROM ghcr.io/savantenvs/base:20261023

ARG COMMIT
ARG REPO_URL
ARG SOURCE_DATE_EPOCH
ARG SANITIZER_FLAGS
ARG COVERAGE_FLAGS=
ARG MAYHEM_JOBS=

# arduino-audio-tools is header-only; on a desktop it needs the Arduino-Emulator (Print/Stream/Client, IPAddress,
# millis, ...). Built once from a pinned commit as a static, uninstrumented library (SPEC 6.2 item 19).
USER root
ARG EMU_COMMIT=350f8dafcf712553fbb285bf68f80900fac41706
RUN [ -z "$SOURCE_DATE_EPOCH" ] || repro-sources-list.sh || true ; \
    apt-get update && apt-get install -yq --no-install-recommends git ca-certificates cmake ninja-build \
 && rm -rf /var/lib/apt/lists/*
RUN d=/opt/toolchains/deps/arduino-emulator-350f8da && mkdir -p "$d" \
 && git clone -q https://github.com/pschatzmann/Arduino-Emulator.git "$d/src" && cd "$d/src" \
 && git checkout -q ${EMU_COMMIT} && git submodule update -q --init --recursive \
 && mkdir /tmp/emu-build && cd /tmp/emu-build \
 && cmake -G Ninja "$d/src" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="-O1 -g -gdwarf-3" -DCMAKE_CXX_FLAGS="-O1 -g -gdwarf-3" \
 && ninja \
 && mkdir -p "$d/lib" && cp libarduino_emulator.a "$d/lib/" \
 && cd / && rm -rf /tmp/emu-build && chmod -R a+rX "$d"
USER mayhem

WORKDIR /mayhem
COPY --chown=2000:2000 . /mayhem

RUN if [ -x /opt/mayhem/ccache/prefill ]; then /opt/mayhem/ccache/prefill bash mayhem/build.sh; \
    else bash mayhem/build.sh; fi

RUN if [ -f mayhem/test.sh ]; then bash mayhem/test.sh; else echo "no mayhem/test.sh — repo ships no functional tests"; fi

LABEL org.opencontainers.image.source="${REPO_URL}" \
      org.opencontainers.image.revision="${COMMIT}" \
      rlenv.tier="commit" \
      rlenv.source_date_epoch="${SOURCE_DATE_EPOCH}" \
      rlenv.user="mayhem"

USER mayhem
WORKDIR /mayhem
