#!/usr/bin/env bash

# AnalogChannel Linux build helper
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
CONFIG="${CONFIG:-Release}"
JOBS="${JOBS:-$(nproc || sysctl -n hw.ncpu || echo 4)}"
JUCE_DIR_ENV="${JUCE_DIR:-}"
PREFIX="${PREFIX:-$HOME/.local}"

echo "== AnalogChannel Linux build =="
echo "Build dir : ${BUILD_DIR}"
echo "Config    : ${CONFIG}"
echo "Jobs      : ${JOBS}"
echo "JUCE_DIR  : ${JUCE_DIR_ENV:-(auto find_package)}"
echo "Prefix    : ${PREFIX}"
echo

cmake -S "$(dirname "${BASH_SOURCE[0]}")/.." -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${CONFIG}" \
    ${JUCE_DIR_ENV:+-DJUCE_DIR="${JUCE_DIR_ENV}"} \
    -DANALOGCHANNEL_STANDALONE=ON

cmake --build "${BUILD_DIR}" --config "${CONFIG}" --parallel "${JOBS}"

echo
echo "Installing to ${PREFIX}..."
cmake --install "${BUILD_DIR}" --config "${CONFIG}" --prefix "${PREFIX}"

echo
echo "Done. VST3 -> ${PREFIX}/lib/vst3/AnalogChannel.vst3"

