# AAG Bar-Ilan Responsa 25 on Ubuntu via Wine/Bottles

## Full Technical Handoff / Reproduction Guide

**Status:** Working production baseline on Ubuntu 26.04 LTS / GNOME Wayland, using Bottles + Soda/Wine 11.0 through XWayland.

**Goal of this document:** A person can hand this file to another AI on a fresh Ubuntu machine and the AI should understand the complete working architecture, the critical binary patch, display/DPI setup, launcher integration, fullscreen fix, touch-scroll helper, known failed approaches, and verification procedure.

---

# 1. Executive summary

The application is an old 32-bit Windows program (`RESPONSA.exe`) installed inside a 64-bit Bottles prefix. It did **not** initially start correctly under Wine 11 because the program contained a broken one-way case-insensitive module-name comparison in its bootstrap. A permanent 2-byte executable patch fixed startup.

The final working architecture is:

```text
Ubuntu 26.04 LTS
GNOME Wayland session
    |
    +-- XWayland :0
          |
          +-- Bottles prefix
                /mnt/data/Bottles/BarIlan__197
          |
          +-- Soda 11.0-10 / Wine 11.0
          |
          +-- RESPONSA.exe (patched)
```

The best visual quality under XWayland requires **Wine DPI = 192**, not 144. The GNOME monitor is physically `1920x1200`, GNOME scale is `1.25`, while XWayland runs a `3072x1920` root surface. Matching Wine to the effective XWayland 2x DPI dramatically improves sharpness.

A touchscreen helper was added. It listens to XInput2 touch events and converts vertical finger drags into direct `WM_VSCROLL` messages to the actual internal MFC content window of Responsa. It does **not** grab the touchscreen, does **not** move the mouse pointer, and does **not** use `ydotool`.

---

# 2. Known-good machine context

The working system used:

```text
Ubuntu Desktop 26.04 LTS
GNOME 50.x
Wayland session
DISPLAY=:0 when using XWayland applications
WAYLAND_DISPLAY=wayland-0
```

Wine/Bottles components:

```text
Bottles Flatpak: com.usebottles.bottles 67.3
Runner: soda-11.0-10
Wine reported version: wine-11.0
```

Known runner path:

```bash
$HOME/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10
```

Bottle path:

```bash
/mnt/data/Bottles/BarIlan__197
```

Application directory:

```bash
/mnt/data/Bottles/BarIlan__197/drive_c/Program Files (x86)/ResponsaCD25
```

Executable:

```bash
/mnt/data/Bottles/BarIlan__197/drive_c/Program Files (x86)/ResponsaCD25/RESPONSA.exe
```

On a new machine the storage path may differ. Keep the same logical layout or update every path in the launcher/helper accordingly.

---

# 3. Host prerequisites

Install basic X11/diagnostic/build utilities used by the final solution:

```bash
sudo apt update
sudo apt install -y \
  xdotool \
  x11-utils \
  x11-xserver-utils \
  xinput \
  wmctrl \
  gcc-mingw-w64-i686
```

Useful verification:

```bash
command -v xdotool
command -v xinput
command -v xprop
command -v xwininfo
command -v i686-w64-mingw32-gcc
```

The final touch solution uses `xinput test-xi2 --root`; therefore `xinput` is required.

---

# 4. Bottles setup

Create a Bottles bottle with an Application environment.

Working bottle characteristics:

```text
Architecture: win64
Environment: Application
Runner: soda-11.0-10
Virtual Desktop: disabled
Window Manager Decorations: enabled
Mouse Warp: enabled
Fullscreen Mouse Capture: disabled
Wayland mode in Bottles: disabled
Renderer: GDI
Custom DPI: 192 (final value)
```

Important historical note: the bottle initially used `custom_dpi: 144`. The final accepted quality is **192**.

Relevant `bottle.yml` values in the working setup included:

```yaml
Runner: soda-11.0-10
Environment: Application
Parameters:
  custom_dpi: 192
  decorated: true
  fullscreen_capture: false
  mouse_warp: true
  renderer: gdi
  virtual_desktop: false
  wayland: false
```

The exact YAML structure can vary with Bottles versions. Use Bottles UI where possible, but verify the final effective value.

---

# 5. Install the application

The original installer was a 32-bit Windows executable (`setup.exe`). A known source location on the original machine was:

```text
/run/media/aag-linux/Local Disk/Softwares/BarIlan/setup.exe
```

That path is machine-specific. On a new machine substitute the actual installer path.

The installer was identified as:

```text
PE32 executable for MS Windows, Intel i386
```

Install it into the bottle and ensure the final directory becomes:

```text
C:\Program Files (x86)\ResponsaCD25
```

which maps to:

```bash
$BOTTLE/drive_c/Program Files (x86)/ResponsaCD25
```

The original installation also had a large installer copy under approximately:

```text
C:\BarIlan-Installer
```

That copy is not required for normal runtime once the program is fully installed.

---

# 6. Runtime dependencies

The successful setup had Microsoft MFC runtime DLLs available in the 32-bit system directory:

```text
mfc42.dll
mfc42u.dll
```

They were manually copied from a known-good Wine prefix into the bottle's `syswow64` directory.

Target directory:

```bash
$BOTTLE/drive_c/windows/syswow64/
```

Also, the Bottles `allfonts` dependency was installed.

Important: the exact donor prefix path for the MFC DLL files was not preserved in the final notes. On another machine, first try installing the equivalent dependency through Bottles/Winetricks (`mfc42`) if available, then verify the DLLs exist in `syswow64`. Do not copy DLLs from untrusted sources.

Verification:

```bash
ls -l \
  "$BOTTLE/drive_c/windows/syswow64/mfc42.dll" \
  "$BOTTLE/drive_c/windows/syswow64/mfc42u.dll"
```

---

# 7. Critical permanent RESPONSA.exe startup patch

## 7.1 Root cause

The application bootstrap contains a broken one-way case-insensitive comparison for module names.

It expected a module name equivalent to:

```text
Kernelbase.dll
```

Wine exposed the loader module name with lowercase initial letter:

```text
kernelbase.dll
```

The application comparison routine incorrectly used an `add 0x20` operation where a lowercase-to-uppercase conversion required subtracting `0x20`.

This caused the application to hang/fail during startup under Wine 11.

## 7.2 Proven patch

Patch location:

```text
Virtual address: 0x640d00
File offset:     0x240100
```

Original bytes:

```text
04 20
```

Patched bytes:

```text
2c 20
```

Semantically:

```text
add al, 0x20
```

became:

```text
sub al, 0x20
```

## 7.3 Known hashes

Original executable SHA256:

```text
7987ad1c1da58f8f4a949451688e987fac44a3c873fe83c1fce4ec5e6a439dcb
```

Patched executable SHA256:

```text
6fcc0381376e5a3a0e57a4b643df08d9a4241141bd8fd3307193d238fede46fe
```

Original backup used on the working machine:

```text
RESPONSA.exe.AAG-ORIGINAL
```

## 7.4 Safe patch procedure

Only patch if the input file hash matches the expected original hash.

```bash
APP="$BOTTLE/drive_c/Program Files (x86)/ResponsaCD25"
EXE="$APP/RESPONSA.exe"

sha256sum "$EXE"
```

If the hash is exactly:

```text
7987ad1c1da58f8f4a949451688e987fac44a3c873fe83c1fce4ec5e6a439dcb
```

then:

```bash
cp -a "$EXE" "$EXE.AAG-ORIGINAL"

python3 - <<'PY'
from pathlib import Path

p = Path("/mnt/data/Bottles/BarIlan__197/drive_c/Program Files (x86)/ResponsaCD25/RESPONSA.exe")
data = bytearray(p.read_bytes())

off = 0x240100
expected = bytes.fromhex("04 20")
patched  = bytes.fromhex("2c 20")

actual = bytes(data[off:off+2])
if actual != expected:
    raise SystemExit(f"Refusing patch: expected {expected.hex()} at 0x{off:x}, got {actual.hex()}")

data[off:off+2] = patched
p.write_bytes(data)
print("PATCH_APPLIED=YES")
PY

sha256sum "$EXE"
```

Expected final hash:

```text
6fcc0381376e5a3a0e57a4b643df08d9a4241141bd8fd3307193d238fede46fe
```

**Do not patch an executable with a different original hash without re-validating the binary layout.**

---

# 8. Hebrew locale

The application was launched with:

```bash
export LANG=he_IL.UTF-8
export LC_ALL=he_IL.UTF-8
```

Wine Intl settings were also verified as Hebrew/Israel:

```text
iCountry    972
Locale      0000040d
LocaleName  he-IL
sCountry    Israel
sLanguage   HEB
```

These locale settings are useful for the application, but they did **not** solve every RTL menu-layout issue. RTL menu experiments are documented later under “Rejected / paused experiments”.

---

# 9. Fullscreen freeze fix

The application originally froze after a small number of menu interactions when maximized/fullscreen under XWayland.

The working fix was to disable Wine's fullscreen/pointer grabbing:
