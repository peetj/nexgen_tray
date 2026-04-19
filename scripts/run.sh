#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
EXE_NAME="${EXE_NAME:-nexgen_tray.exe}"

"${BUILD_DIR}/${EXE_NAME}" "$@"
