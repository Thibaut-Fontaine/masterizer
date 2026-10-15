#!/usr/bin/env sh
# Configure, build and test on Linux/macOS.
set -eu

cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
echo "Done. CLI: ./build/linux-debug/app/masterizer_cli <ref.wav> <src.wav> <out.wav> [intensity]"
