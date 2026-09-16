                LONG area = w * h;

                if (w > 200 && h > 100 && area > best_area) {
                    best = child;
                    best_area = area;
                }
            }
        }

        scan(child);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

static BOOL CALLBACK enum_top(HWND hwnd, LPARAM lp)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    if (is_responsa_pid(pid))
        scan(hwnd);

    return TRUE;
}

static HWND find_target(void)
{
    best = NULL;
    best_area = 0;
    EnumWindows(enum_top, 0);
    return best;
}

int main(void)
{
    char line[32];

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("SCROLLBRIDGE_READY\n");

    while (fgets(line, sizeof(line), stdin)) {
        HWND hwnd = find_target();

        if (!hwnd) {
            printf("NO_TARGET\n");
            continue;
        }

        if (line[0] == 'D')
            SendMessageW(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);

        else if (line[0] == 'U')
            SendMessageW(hwnd, WM_VSCROLL, SB_LINEUP, 0);

        else if (line[0] == 'Q')
            break;
    }

    return 0;
}
```

Compile:

```bash
PERM="$HOME/.local/share/barilan-responsa/touchscroll"

i686-w64-mingw32-gcc \
  -O2 \
  "$PERM/scrollbridge.c" \
  -o "$PERM/scrollbridge.exe" \
  -luser32 -lkernel32
```

## 16.2 `touchscroll.py`

The accepted user feel is deliberately conservative, not kinetic/inertial.

Final tuning:

```text
PIXELS_PER_STEP = 65.0
DEADZONE        = 6.0
```

Accepted implementation:

```python
#!/usr/bin/env python3

import os
import re
import signal
import subprocess
import sys

BOTTLE = "/mnt/data/Bottles/BarIlan__197"
RUNNER = os.path.expanduser(
    "~/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10"
)
WORK = os.path.dirname(os.path.abspath(__file__))
BRIDGE = os.path.join(WORK, "scrollbridge.exe")

env = os.environ.copy()
env["DISPLAY"] = ":0"
env["WINEPREFIX"] = BOTTLE
env["WINEDEBUG"] = "-all"

bridge = subprocess.Popen(
    [os.path.join(RUNNER, "bin", "wine"), BRIDGE],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.DEVNULL,
    text=True,
    bufsize=1,
    env=env,
)

print(bridge.stdout.readline().strip())

xi = subprocess.Popen(
    ["xinput", "test-xi2", "--root"],
    stdout=subprocess.PIPE,
    stderr=subprocess.DEVNULL,
    text=True,
    bufsize=1,
    env={**os.environ, "DISPLAY": ":0"},
)

event_re = re.compile(r"EVENT type\s+\d+\s+\(([^)]+)\)")
root_re = re.compile(r"root:\s+[-0-9.]+/([-0-9.]+)")

active = False
event = None
last_y = None
accum = 0.0

PIXELS_PER_STEP = 65.0
DEADZONE = 6.0

def send(cmd):
    if bridge.poll() is None:
        bridge.stdin.write(cmd + "\n")
        bridge.stdin.flush()

def cleanup(*_):
    try:
        xi.terminate()
    except Exception:
        pass

    try:
        send("Q")
        bridge.terminate()
    except Exception:
        pass

    print("\nTOUCH_SCROLL_STOPPED")
    sys.exit(0)

signal.signal(signal.SIGINT, cleanup)
signal.signal(signal.SIGTERM, cleanup)

for raw in xi.stdout:
    line = raw.strip()

    m = event_re.search(line)
    if m:
        event = m.group(1)

        if event == "TouchBegin":
            active = True
            last_y = None
            accum = 0.0

        elif event == "TouchEnd":
            active = False
            last_y = None
            accum = 0.0

        continue

    if not active or event not in ("TouchBegin", "TouchUpdate"):
        continue

    m = root_re.search(line)
    if not m:
        continue

    y = float(m.group(1))

    if last_y is None:
        last_y = y
        continue

    dy = y - last_y
    last_y = y

    if abs(dy) < DEADZONE:
        continue

    accum += dy

    if accum <= -PIXELS_PER_STEP:
        send("D")
        accum += PIXELS_PER_STEP

    elif accum >= PIXELS_PER_STEP:
        send("U")
        accum -= PIXELS_PER_STEP

cleanup()
```

This is intentionally discrete because Responsa's internal scroll model itself is discrete. Attempts to make it pixel-smooth are documented later and were rejected.

---

# 17. Main launcher

Production launcher path:

```bash
$HOME/.local/bin/barilan-responsa
```

Recommended final launcher:

```bash
#!/usr/bin/env bash
set -Eeuo pipefail

BOTTLE="/mnt/data/Bottles/BarIlan__197"
APP="$BOTTLE/drive_c/Program Files (x86)/ResponsaCD25"
RUNNER="$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10"
LOG="$HOME/.local/state/barilan-responsa.log"

mkdir -p "$(dirname "$LOG")"
unset WINEARCH
export LANG=he_IL.UTF-8
export LC_ALL=he_IL.UTF-8

# Do not create a duplicate application instance.
if pgrep -f 'RESPONSA\.exe' >/dev/null; then
    exit 0
fi

cd "$APP"

# AAG_TOUCH_SCROLL_FINAL
pkill -TERM -f 'touchscroll.py' 2>/dev/null || true
nohup python3 "$HOME/.local/share/barilan-responsa/touchscroll/touchscroll.py" \
    >/dev/null 2>&1 &

# AAG_ALT_TAB_HELPER
"$HOME/.local/bin/barilan-hide-alt-tab-helper" >/dev/null 2>&1 &

exec env \
    DISPLAY=:0 \
    WAYLAND_DISPLAY=wayland-0 \
    XDG_SESSION_TYPE=wayland \
    WINEPREFIX="$BOTTLE" \
    WINEDEBUG=-all \
    "$RUNNER/bin/wine" ./RESPONSA.exe >>"$LOG" 2>&1
```

Notes:

1. `DISPLAY=:0` is important because the accepted final mode is XWayland.
2. The touch helper starts before the application and dynamically waits for a suitable target HWND when touch input begins.
3. `pkill` prevents duplicate helper instances.
4. Locale is forced to Hebrew.

Make executable:

```bash
chmod +x "$HOME/.local/bin/barilan-responsa"
```

---

# 18. Desktop launcher

Desktop file:

```bash
$HOME/.local/share/applications/aag-responsa25.desktop
```

Example:

```ini
[Desktop Entry]
Type=Application
Name=Responsa 25
Exec=/home/USER/.local/bin/barilan-responsa
Icon=E411_RESPONSA.0
Terminal=false
StartupNotify=true
StartupWMClass=responsa.exe
Categories=Office;
```

Replace `/home/USER` with the actual home directory.

A clean custom desktop entry was used because the Wine-generated nested desktop entry caused launch problems. The old generated entry was hidden/removed from normal use.

Refresh desktop entries if needed:

```bash
update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
```

---

# 19. Final verification checklist

## 19.1 Executable integrity

```bash
sha256sum \
  "$BOTTLE/drive_c/Program Files (x86)/ResponsaCD25/RESPONSA.exe"
```

Expected:

```text
6fcc0381376e5a3a0e57a4b643df08d9a4241141bd8fd3307193d238fede46fe
```

## 19.2 DPI

```bash
WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wine" reg query \
'HKCU\Control Panel\Desktop' /v LogPixels
```

Expected:

```text
0xc0
```

## 19.3 Fullscreen input settings

```bash
WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wine" reg query \
'HKCU\Software\Wine\X11 Driver'
```

Expected:

```text
GrabFullscreen = N
GrabPointer    = N
```

## 19.4 Touch helper files

```bash
find "$HOME/.local/share/barilan-responsa/touchscroll" \
  -maxdepth 1 -type f -printf '%f\n' | sort
```

Expected exactly:

```text
scrollbridge.c
scrollbridge.exe
touchscroll.py
```

## 19.5 Running processes after launching from the icon

```bash
pgrep -af 'RESPONSA\.exe|touchscroll.py|scrollbridge.exe|xinput test-xi2'
```
