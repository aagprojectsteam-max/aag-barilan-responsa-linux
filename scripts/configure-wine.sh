#!/usr/bin/env bash
set -euo pipefail
BOTTLE="${1:?Usage: configure-wine.sh BOTTLE_PATH [RUNNER_PATH]}"
RUNNER="${2:-$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10}"
WINE="$RUNNER/bin/wine"
WINESERVER="$RUNNER/bin/wineserver"
DISPLAY="${DISPLAY:-:0}"
export DISPLAY WINEPREFIX="$BOTTLE"

"$WINE" reg add 'HKCU\Control Panel\Desktop' /v LogPixels /t REG_DWORD /d 192 /f
"$WINE" reg add 'HKCU\Software\Wine\X11 Driver' /v GrabFullscreen /t REG_SZ /d N /f
"$WINE" reg add 'HKCU\Software\Wine\X11 Driver' /v GrabPointer /t REG_SZ /d N /f
"$WINE" reg add 'HKCU\Software\Wine\Direct3D' /v renderer /t REG_SZ /d gdi /f
"$WINESERVER" -k 2>/dev/null || true

echo "CONFIGURED=YES"
"$WINE" reg query 'HKCU\Control Panel\Desktop' /v LogPixels || true
"$WINE" reg query 'HKCU\Software\Wine\X11 Driver' || true
"$WINE" reg query 'HKCU\Software\Wine\Direct3D' || true
