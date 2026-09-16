#!/usr/bin/env bash
set -Eeuo pipefail

BOTTLE="${AAG_RESPONSA_BOTTLE:-/mnt/data/Bottles/BarIlan__197}"
APP="${AAG_RESPONSA_APP:-$BOTTLE/drive_c/Program Files (x86)/ResponsaCD25}"
RUNNER="${AAG_RESPONSA_RUNNER:-$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10}"
LOG="${AAG_RESPONSA_LOG:-$HOME/.local/state/barilan-responsa.log}"
TOUCH="$HOME/.local/share/barilan-responsa/touchscroll/touchscroll.py"

mkdir -p "$(dirname "$LOG")"
unset WINEARCH
export LANG="${LANG:-he_IL.UTF-8}"
export LC_ALL="${LC_ALL:-he_IL.UTF-8}"

if pgrep -f 'RESPONSA\.exe' >/dev/null; then
    exit 0
fi

cd "$APP"
pkill -TERM -f '/barilan-responsa/touchscroll/touchscroll.py' 2>/dev/null || true
nohup python3 "$TOUCH" --bottle "$BOTTLE" --runner "$RUNNER" >/dev/null 2>&1 &

# Optional: if installed, hide the 1x1 Wine helper window from Alt+Tab.
if [ -x "$HOME/.local/bin/barilan-hide-alt-tab-helper" ]; then
    "$HOME/.local/bin/barilan-hide-alt-tab-helper" >/dev/null 2>&1 &
fi

exec env \
    DISPLAY="${DISPLAY:-:0}" \
    WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}" \
    XDG_SESSION_TYPE="${XDG_SESSION_TYPE:-wayland}" \
    WINEPREFIX="$BOTTLE" \
    WINEDEBUG=-all \
    "$RUNNER/bin/wine" ./RESPONSA.exe >>"$LOG" 2>&1
