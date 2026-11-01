#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --parallel
exec "$BUILD_DIR/raildiag-gateway" --bind 127.0.0.1 --port "${PORT:-9100}" --log "${LOG_FILE:-$ROOT/diagnostics.csv}"
