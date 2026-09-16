Expected concepts:

```text
RESPONSA.exe
touchscroll.py from ~/.local/share/barilan-responsa/touchscroll/
scrollbridge.exe from ~/.local/share/barilan-responsa/touchscroll/
xinput test-xi2 --root
```

## 19.6 No stale Downloads dependency

```bash
grep -R 'Downloads/AAG-Responsa' \
  "$HOME/.local/bin/barilan-responsa" \
  "$HOME/.local/share/barilan-responsa/touchscroll" \
  2>/dev/null || echo PASS_NO_OLD_REFERENCES
```

Expected:

```text
PASS_NO_OLD_REFERENCES
```

---

# 20. Final accepted behavior

The accepted production state is:

```text
Application launches from Ubuntu icon: PASS
Startup under Wine 11 / Soda:         PASS
Fullscreen usable:                    PASS
No fullscreen freeze:                 PASS
Visual sharpness at DPI 192:          PASS / best tested
Alt+Tab ghost hidden:                 PASS
Touch tap/select:                     PASS
Touch vertical scroll:                PASS
Touch helper no device grab:          PASS
Touch helper no pointer warp:         PASS
Touch helper persistent path:         PASS
No dependency on Downloads:           PASS
```

The touch scroll is intentionally **controlled and discrete**, not kinetic/pixel-smooth. This is a limitation of the application's own internal scroll model.

---

# 21. Rejected / failed experiments — important for future AI agents

Do not repeat these unless deliberately researching a new Wine version.

## 21.1 Native Wine Wayland as production mode

Result:

```text
Sharp rendering: excellent
Pointer/click coordinate alignment: bad
Fullscreen: bad / unavailable
```

Decision: rejected for production.

## 21.2 Wine devel 11.17

Tested as an A/B replacement for Soda 11.0.

Result:

```text
No useful RTL improvement
Fullscreen disappeared / regressed
```

Decision: return to Soda 11.0-10.

## 21.3 Touch helper using evdev grab + ydotool absolute movement

Result:

```text
Touch stopped behaving normally
Mouse pointer jumped toward bottom-right
```

Decision: completely rejected.

## 21.4 xdotool wheel events

Tested:

```bash
xdotool click 4
xdotool click 5
```

Responsa did not scroll.

Decision: mouse-wheel emulation is not the solution.

## 21.5 Keyboard injection experiments

Direct generic keyboard injection was abandoned after a test caused terminal/session trouble and was unnecessary once the internal MFC scrollbar was found.

Decision: do not use keyboard emulation for touch scrolling.

## 21.6 `SB_THUMBPOSITION` / direct thumb positioning

The application did not respond usefully to direct thumb-position control.

Decision: use `SB_LINEUP` / `SB_LINEDOWN` only.

## 21.7 Smooth velocity / kinetic scrolling

Attempts to convert finger velocity into rapid line-scroll commands felt too fast and unlike direct touch manipulation.

Decision: rejected.

## 21.8 Pixel-drag GDI inside the application HWND

A `BitBlt` experiment attempted to move the content pixels inside the internal MFC HWND.

Result: no useful visible direct manipulation.

Decision: rejected.

## 21.9 External overlay direct manipulation

A snapshot overlay could be displayed, proving the concept technically, but when tied to live finger coordinates it became unstable because of Wine/XWayland coordinate scaling mismatch. The view became huge and moved wildly.

Decision: rejected. Do not enable overlay code in production.

---

# 22. RTL menu experiments — not part of the final requirement

Several attempts were made to improve top-level and popup RTL menus.

What worked partially:

```text
- reversing top-level menu items
- MFT_RIGHTJUSTIFY on the first top-level item
```

What did not produce an acceptable final popup RTL implementation:

```text
- recursive MFT_RIGHTORDER
- popup #32768 style polling
- WS_EX_LAYOUTRTL / WS_EX_RTLREADING experiments
- IAT hooks on InsertMenuW / AppendMenuW
- temporary process layout changes during menu loops
- CBT popup hooks
```

One experiment patched an internal F66 assignment and caused the application UI to become English / internally mirrored. It was reverted.

Current decision: RTL popup work was paused because the user preferred spending time on stability, quality, fullscreen, and touch.

Do not reapply the F66 patch.

---

# 23. Useful diagnostics

## Session

```bash
echo "$XDG_SESSION_TYPE"
echo "$DISPLAY"
echo "$WAYLAND_DISPLAY"
```

Expected host session:

```text
wayland
```

When Responsa runs through XWayland its process environment should contain:

```text
DISPLAY=:0
XDG_SESSION_TYPE=wayland
WAYLAND_DISPLAY=wayland-0
```

## XWayland geometry

```bash
DISPLAY=:0 xdpyinfo | grep -E 'dimensions:|resolution:'
DISPLAY=:0 xrandr --current | head -20
DISPLAY=:0 xrdb -query | grep -i dpi
```

Known-good system produced approximately:

```text
dimensions: 3072x1920
Xft.dpi: 192
```

## GNOME logical monitor scale

```bash
gdbus call \
  --session \
  --dest org.gnome.Mutter.DisplayConfig \
  --object-path /org/gnome/Mutter/DisplayConfig \
  --method org.gnome.Mutter.DisplayConfig.GetCurrentState
```

Known-good system reported monitor scale:

```text
1.25
```

---

# 24. Portable installation strategy for a fresh machine

For a clean deployment, an AI should follow this order:

1. Install Bottles and the required host packages.
2. Create an Application bottle and select Soda 11.0-10 or the exact compatible runner available in the repository/package bundle.
3. Install Responsa 25 from the legitimate installer/media.
4. Ensure `mfc42.dll` and `mfc42u.dll` are available in the 32-bit Wine system directory.
5. Install the required fonts (`allfonts` was used on the accepted machine).
6. Verify the original `RESPONSA.exe` SHA256 before binary patching.
7. Apply the exact 2-byte startup patch only to the known original hash.
8. Configure Hebrew locale.
9. Configure X11 driver `GrabFullscreen=N`, `GrabPointer=N`.
10. Set renderer to GDI.
11. Set Wine/Bottles DPI to 192.
12. Keep Bottles native Wayland disabled; run through XWayland.
13. Install the persistent touch helper into `~/.local/share/barilan-responsa/touchscroll/`.
14. Compile `scrollbridge.exe` using `i686-w64-mingw32-gcc`.
15. Create `~/.local/bin/barilan-responsa`.
16. Install the Alt+Tab ghost helper if needed.
17. Create a clean desktop entry.
18. Launch only through the custom launcher/icon.
19. Run the verification checklist.
20. Do not delete installation backups until all acceptance checks pass.

---

# 25. GitHub publication recommendations

Do **not** upload proprietary application binaries or DLLs unless redistribution rights are explicitly confirmed.

Safe public repository contents should include only original tooling/documentation such as:

```text
README.md
HANDOFF.md
scripts/install-touch-helper.sh
scripts/configure-wine.sh
src/scrollbridge.c
src/touchscroll.py
examples/barilan-responsa-launcher.sh
examples/aag-responsa25.desktop
LICENSE
SECURITY.md
CHANGELOG.md
```

Do not publish:

```text
RESPONSA.exe
setup.exe
mfc42.dll
mfc42u.dll
application database/content files
commercial installer files
```

The binary patch can be documented as an offset + expected bytes + hashes, or implemented by a patch script that requires the user to supply their own legally obtained executable.

Recommended patch script policy:

```text
- verify exact original SHA256
- verify exact bytes at the patch offset
- create backup
- patch two bytes
- verify final SHA256
- fail closed on any mismatch
```

---

# 26. Suggested repository structure

```text
barilan-responsa-linux/
├── README.md
├── HANDOFF.md
├── LICENSE
├── CHANGELOG.md
├── SECURITY.md
├── src/
│   ├── scrollbridge.c
│   └── touchscroll.py
├── scripts/
│   ├── patch-responsa.py
│   ├── configure-wine.sh
│   ├── install-touch-helper.sh
│   └── verify-install.sh
└── examples/
    ├── barilan-responsa-launcher.sh
    └── aag-responsa25.desktop
```

---

# 27. Final production baseline snapshot

```text
BOTTLE=/mnt/data/Bottles/BarIlan__197
APP=/mnt/data/Bottles/BarIlan__197/drive_c/Program Files (x86)/ResponsaCD25
RUNNER=$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10
WINE_VERSION=11.0
MODE=XWAYLAND
WINE_DPI=192
RENDERER=GDI
GRAB_FULLSCREEN=N
GRAB_POINTER=N
RESPONSA_SHA256=6fcc0381376e5a3a0e57a4b643df08d9a4241141bd8fd3307193d238fede46fe
TOUCH_PIXELS_PER_STEP=65
TOUCH_DEADZONE=6
TOUCH_GRAB=NO
TOUCH_POINTER_WARP=NO
TOUCH_SCROLL_TRANSPORT=WM_VSCROLL
TOUCH_HELPER=$HOME/.local/share/barilan-responsa/touchscroll/touchscroll.py
TOUCH_BRIDGE=$HOME/.local/share/barilan-responsa/touchscroll/scrollbridge.exe
LAUNCHER=$HOME/.local/bin/barilan-responsa
STATUS=WORKING_ACCEPTED
```

---

# 28. Important limitations

1. The application is old and uses an MFC scroll model with coarse logical positions. The touch helper therefore cannot provide true modern pixel-smooth kinetic scrolling without a much more invasive rendering/input layer.
2. Native Wine Wayland was sharper but failed pointer-coordinate/fullscreen acceptance on the tested setup.
3. The 2-byte executable patch is version-specific and must never be blindly applied to an unknown binary.
4. The DPI 192 recommendation was empirically optimal on the tested GNOME 125% / XWayland 3072x1920 configuration. A machine with a different monitor scale may require re-testing, though 192 is the accepted baseline for this documented environment.
5. Internal Win32 HWND values change between launches; never hard-code the observed `0x0002025c` value.

---

# 29. Handoff instruction to another AI

If this document is given to another AI, the intended instruction is:

> Reproduce the accepted Bar-Ilan Responsa 25 Ubuntu/Wine setup described here. Treat the documented production baseline as authoritative. Do not repeat rejected experiments unless necessary. Before changing anything, inspect the current machine for existing Bottles/Wine installations and preserve backups. Verify hashes before patching. Keep the application on XWayland, configure DPI 192, use Soda/Wine 11.0-compatible runtime, preserve the fullscreen registry fix, compile and install the touch-scroll bridge, create the launcher and desktop file, and run every final verification check. Do not redistribute proprietary application binaries; require the user to provide their own legitimate installer/media.

---

# 30. Acceptance statement

The original machine reached a working accepted state after real user testing:

```text
STARTUP=PASS
FULLSCREEN=PASS
DISPLAY_SHARPNESS_192_DPI=PASS
ALT_TAB_GHOST_FIX=PASS
TOUCH_SCROLL=PASS
TOUCH_HELPER_PERSISTENT=PASS
NO_DOWNLOAD_RUNTIME_DEPENDENCY=PASS
OLD_TOUCH_EXPERIMENTS_CLEANED=PASS
```

This document records the complete technical path needed to reproduce that baseline on another Ubuntu system.
