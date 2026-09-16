#!/usr/bin/env bash
set -u
BOTTLE="${1:?Usage: verify-install.sh BOTTLE_PATH [RUNNER_PATH]}"
RUNNER="${2:-$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10}"
EXE="$BOTTLE/drive_c/Program Files (x86)/ResponsaCD25/RESPONSA.exe"
PERM="$HOME/.local/share/barilan-responsa/touchscroll"

echo "=== Executable ==="
sha256sum "$EXE" 2>/dev/null || true

echo "=== DPI ==="
DISPLAY="${DISPLAY:-:0}" WINEPREFIX="$BOTTLE" "$RUNNER/bin/wine" reg query 'HKCU\Control Panel\Desktop' /v LogPixels 2>/dev/null || true

echo "=== X11 Driver ==="
DISPLAY="${DISPLAY:-:0}" WINEPREFIX="$BOTTLE" "$RUNNER/bin/wine" reg query 'HKCU\Software\Wine\X11 Driver' 2>/dev/null || true

echo "=== Touch helper files ==="
find "$PERM" -maxdepth 1 -type f -printf '%f\n' 2>/dev/null | sort || true

echo "=== Processes ==="
pgrep -af 'RESPONSA\.exe|touchscroll.py|scrollbridge.exe|xinput test-xi2' || true
