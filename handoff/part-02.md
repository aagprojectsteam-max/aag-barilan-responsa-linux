```bash
BOTTLE="/mnt/data/Bottles/BarIlan__197"
RUNNER="$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10"

WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wine" reg add \
'HKCU\Software\Wine\X11 Driver' \
/v GrabFullscreen /t REG_SZ /d N /f

WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wine" reg add \
'HKCU\Software\Wine\X11 Driver' \
/v GrabPointer /t REG_SZ /d N /f
```

Verification:

```bash
WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wine" reg query \
'HKCU\Software\Wine\X11 Driver'
```

Expected:

```text
GrabFullscreen    REG_SZ    N
GrabPointer       REG_SZ    N
```

This was accepted as working.

---

# 10. Display sharpness / DPI solution

## 10.1 Important observed display state

GNOME/Mutter reported:

```text
Physical monitor mode: 1920x1200
GNOME logical scale:    1.25
```

XWayland reported:

```text
Root dimensions: 3072x1920
xdpyinfo DPI:     96x96
Xft.dpi:          192
```

Therefore XWayland's effective surface dimensions are 1.6x the physical mode and 2x the GNOME logical dimensions.

## 10.2 DPI experiments

Observed quality scores:

```text
168 DPI -> 5/10, not good
180 DPI -> 8/10, good
192 DPI -> 10/10, very good
```

Final accepted Wine DPI:

```text
192
```

Set it:

```bash
BOTTLE="/mnt/data/Bottles/BarIlan__197"
RUNNER="$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10"

DISPLAY=:0 \
WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wine" reg add \
'HKCU\Control Panel\Desktop' \
/v LogPixels \
/t REG_DWORD \
/d 192 \
/f

WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wineserver" -k 2>/dev/null || true
```

Verification:

```bash
WINEPREFIX="$BOTTLE" \
"$RUNNER/bin/wine" reg query \
'HKCU\Control Panel\Desktop' /v LogPixels
```

Expected:

```text
LogPixels    REG_DWORD    0xc0
```

`0xc0` = decimal 192.

Also set Bottles `custom_dpi: 192` so Bottles does not later overwrite the registry value.

---

# 11. Native Wayland experiment — DO NOT use as final mode

A direct native Wine Wayland test was performed by **unsetting DISPLAY**:

```bash
unset DISPLAY
LANG=he_IL.UTF-8 \
LC_ALL=he_IL.UTF-8 \
WAYLAND_DISPLAY=wayland-0 \
XDG_SESSION_TYPE=wayland \
WINEPREFIX="$BOTTLE" \
WINEDEBUG=-all \
"$RUNNER/bin/wine" ./RESPONSA.exe
```

Positive result:

```text
Visual quality was excellent / extremely sharp.
```

But it introduced two unacceptable problems:

```text
1. Pointer position and click position did not align correctly.
2. Fullscreen did not work correctly.
```

Therefore the final production mode remains **XWayland**, not native Wine Wayland.

---

# 12. Renderer

The final working bottle uses:

```text
renderer = gdi
```

Registry:

```text
HKCU\Software\Wine\Direct3D
renderer = gdi
```

This is part of the accepted baseline.

---

# 13. Alt+Tab ghost-window fix

Responsa created an extra invisible 1x1 X11 window that appeared as a separate Alt+Tab entry.

Observed ghost characteristics included:

```text
Size: 1x1
Map state: IsViewable
_NET_WM_WINDOW_TYPE_NORMAL
Same PID as RESPONSA.exe
No title
```

Changing the ghost to a utility window removed it from Alt+Tab:

```bash
xprop -id "$WID" -f _NET_WM_WINDOW_TYPE 32a \
  -set _NET_WM_WINDOW_TYPE _NET_WM_WINDOW_TYPE_UTILITY
```

A permanent helper was installed at:

```bash
$HOME/.local/bin/barilan-hide-alt-tab-helper
```

The main launcher starts it before Responsa.

The helper identifies the 1x1 titleless visible window owned by the current Responsa PID and marks it as utility / skip-taskbar as appropriate.

Keep this helper in the final installation if the ghost window reproduces on the target machine.

---

# 14. Touchscreen architecture

## 14.1 Hardware verification

Touchscreen:

```text
ELAN2513:00 04F3:4284
/dev/input/event4
Capabilities: touch
Size: 303x187mm
```

libinput produced correct event sequences:

```text
TOUCH_DOWN
TOUCH_MOTION
...
TOUCH_UP
```

XWayland/XInput2 also produced:

```text
RawTouchBegin
TouchBegin
RawTouchUpdate
TouchUpdate
...
RawTouchEnd
TouchEnd
```

Therefore:

```text
Hardware = OK
libinput = OK
XWayland XI2 touch = OK
```

The application itself does not naturally convert those drags into smooth scrolling.

## 14.2 Important failed touch approach — never repeat

A first helper used:

```text
evdev + dev.grab() + ydotool absolute mouse movement
```

Result:

```text
- touchscreen interaction broke
- mouse pointer repeatedly jumped toward bottom-right
```

This approach was removed and must **not** be repeated.

Do not use EVIOCGRAB for this application.
Do not use ydotool absolute pointer movement for this feature.

---

# 15. Discovering the real internal scroll window

X11 sees only one top-level Responsa window and no X11 child windows. The actual application UI is an MFC hierarchy of internal Win32 HWNDs inside Wine.

A small 32-bit Win32 probe was compiled with MinGW and run inside the same Wine prefix.

The primary content view was identified as an MFC child window with:

```text
CLASS=Afx:00400000:82b:00010058:01900020:00000000
WS_VSCROLL=1
RECT approximately 1518x831 Wine logical pixels
```

During one run it had:

```text
HWND=0002025c
VSCROLL min=0 max=38 pos=19
```

The HWND is **not stable between program launches**, so the final helper does not hard-code it. It dynamically searches for the largest visible Responsa child HWND that:

```text
- belongs to RESPONSA.exe
- has WS_VSCROLL
- has a real scroll range
- is wider than 200 pixels
- is taller than 100 pixels
- has the largest area among candidates
```

A direct test proved:

```text
BEFORE       pos=19
3x LINEDOWN  pos=22
3x LINEUP    pos=19
```

Thus direct `WM_VSCROLL` is the proven mechanism.

---

# 16. Final touch-scroll implementation

Final persistent location:

```bash
$HOME/.local/share/barilan-responsa/touchscroll/
```

Final files:

```text
scrollbridge.c
scrollbridge.exe
touchscroll.py
```

No production component depends on `~/Downloads`.

## 16.1 `scrollbridge.c`

This is the accepted bridge implementation:

```c
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static HWND best = NULL;
static LONG best_area = 0;

static int is_responsa_pid(DWORD pid)
{
    HANDLE h;
    WCHAR path[2048];
    DWORD len = 2048;
    WCHAR *base;

    h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return 0;

    if (!QueryFullProcessImageNameW(h, 0, path, &len)) {
        CloseHandle(h);
        return 0;
    }

    CloseHandle(h);

    base = wcsrchr(path, L'\\');
    base = base ? base + 1 : path;

    return _wcsicmp(base, L"RESPONSA.exe") == 0;
}

static void scan(HWND parent)
{
    HWND child = GetWindow(parent, GW_CHILD);

    while (child) {
        LONG_PTR style = GetWindowLongPtrW(child, GWL_STYLE);

        if (IsWindowVisible(child) && (style & WS_VSCROLL)) {
            RECT r;
            SCROLLINFO si;

            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;

            if (GetWindowRect(child, &r) &&
                GetScrollInfo(child, SB_VERT, &si) &&
                si.nMax > si.nMin) {

                LONG w = r.right - r.left;
                LONG h = r.bottom - r.top;
