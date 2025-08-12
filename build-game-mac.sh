#!/usr/bin/env bash
set -euo pipefail

CONFIG="Debug"
if [[ ${1:-} == "--config" && -n ${2:-} ]]; then
  CONFIG="$2"
  shift 2
fi

if [[ "$CONFIG" != "Debug" && "$CONFIG" != "Release" ]]; then
  echo "Unknown config: $CONFIG (use Debug or Release)" >&2
  exit 1
fi

VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
if [[ ! -d "$VCPKG_ROOT" ]]; then
  echo "VCPKG_ROOT not found at $VCPKG_ROOT. Install vcpkg or set VCPKG_ROOT." >&2
  exit 1
fi

ARCH=$(uname -m)
if [[ "$ARCH" == "arm64" ]]; then
  TRIPLET="arm64-osx"
else
  TRIPLET="x64-osx"
fi

if command -v ninja >/dev/null 2>&1; then
  GENERATOR="Ninja"
else
  GENERATOR="Unix Makefiles"
fi

BUILD_DIR="build/macos-${TRIPLET}-${CONFIG}"
echo "Config: $CONFIG | Arch: $ARCH -> $TRIPLET | Generator: $GENERATOR"
echo "Build dir: $BUILD_DIR"

cmake -S . -B "$BUILD_DIR" \
  -G "$GENERATOR" \
  -DCMAKE_BUILD_TYPE="$CONFIG" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET="$TRIPLET"

cmake --build "$BUILD_DIR" --config "$CONFIG" -j

echo "\nBuilt successfully. Run the game with:"
echo "  $BUILD_DIR/BrotatoGame"


