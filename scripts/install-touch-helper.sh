#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="${1:-$HOME/.local/share/barilan-responsa/touchscroll}"
mkdir -p "$DEST"
cp "$ROOT/src/scrollbridge.c" "$DEST/scrollbridge.c"
cp "$ROOT/src/touchscroll.py" "$DEST/touchscroll.py"
chmod +x "$DEST/touchscroll.py"
command -v i686-w64-mingw32-gcc >/dev/null || { echo "Missing i686-w64-mingw32-gcc" >&2; exit 1; }
i686-w64-mingw32-gcc -O2 "$DEST/scrollbridge.c" -o "$DEST/scrollbridge.exe" -luser32 -lkernel32
chmod +x "$DEST/scrollbridge.exe"
echo "INSTALLED=$DEST"
