#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./scripts/build.sh
#   UPDATE=0 ./scripts/build.sh
#   QT_PREFIX=/c/Qt/6.11.0/mingw_64 ./scripts/build.sh

UPDATE="${UPDATE:-1}"
QT_PREFIX="${QT_PREFIX:-/c/Qt/6.11.0/mingw_64}"
BUILD_DIR="${BUILD_DIR:-build}"
GENERATOR="${GENERATOR:-Ninja}"

if [[ "$UPDATE" == "1" ]]; then
  "$(dirname "$0")/update.sh" || true
fi

cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_PREFIX_PATH="$QT_PREFIX"
cmake --build "$BUILD_DIR"
