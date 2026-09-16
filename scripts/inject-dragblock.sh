#!/usr/bin/env bash
set -u

BOTTLE="${AAG_RESPONSA_BOTTLE:-/mnt/data/Bottles/BarIlan__197}"
RUNNER="${AAG_RESPONSA_RUNNER:-$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10}"
BASE="${AAG_RESPONSA_DRAGBLOCK_DIR:-$HOME/.local/share/barilan-responsa/dragblock}"
DLL="$BASE/AAG-Responsa-DragBlock-Precise.dll"
INJECTOR="$BASE/inject.exe"

for _ in $(seq 1 80); do
    if pgrep -f 'RESPONSA\.exe' >/dev/null 2>&1; then
        break
    fi
    sleep 0.25
done

if ! pgrep -f 'RESPONSA\.exe' >/dev/null 2>&1; then
    exit 0
fi

if [ ! -f "$DLL" ] || [ ! -f "$INJECTOR" ]; then
    exit 0
fi

DLL_WIN="$(WINEPREFIX="$BOTTLE" "$RUNNER/bin/winepath" -w "$DLL" 2>/dev/null || true)"
[ -n "$DLL_WIN" ] || exit 0

DISPLAY="${DISPLAY:-:0}" \
WINEPREFIX="$BOTTLE" \
WINEDEBUG=-all \
"$RUNNER/bin/wine" "$INJECTOR" "$DLL_WIN" >/dev/null 2>&1 || true
