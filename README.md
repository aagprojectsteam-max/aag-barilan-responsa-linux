# Bar-Ilan Responsa 25 on Ubuntu via Wine/Bottles

Community technical notes and helper tooling for running a user-supplied installation of Bar-Ilan Responsa 25 on Ubuntu with Bottles/Wine.

## What this repository contains

- a version-locked 2-byte startup patcher for one verified `RESPONSA.exe` build;
- Wine/XWayland configuration used by the accepted setup;
- a touchscreen-to-`WM_VSCROLL` helper for the application's internal MFC content window;
- an optional precise content-view drag/long-press filter (`src/dragblock-precise.c`);
- launcher and desktop-entry examples;
- a complete reproduction and troubleshooting handoff in [`HANDOFF.md`](HANDOFF.md).

## Tested baseline

- Ubuntu Desktop 26.04 LTS / GNOME Wayland
- Bottles 67.3
- Soda 11.0-10 / Wine 11.0
- XWayland mode
- Wine DPI `192`
- GDI renderer
- `GrabFullscreen=N`, `GrabPointer=N`

The tested machine used GNOME scale `1.25`; XWayland exposed `3072x1920` for a physical `1920x1200` panel. DPI 192 was empirically the sharpest accepted setting on that environment.

## Important legal note

This repository does **not** contain the commercial application, installer, content/database files, or Microsoft runtime DLLs. You must provide your own legitimately obtained software/media. The patcher refuses to touch executables that do not match the exact verified SHA256.

## Quick start

1. Install Bottles and create an Application bottle using a Wine/Soda 11-compatible runner.
2. Install Responsa 25 from your own media into the bottle.
3. Install prerequisites:

```bash
sudo apt update
sudo apt install -y xdotool x11-utils x11-xserver-utils xinput wmctrl gcc-mingw-w64-i686
```

4. Patch only the verified executable build:

```bash
python3 scripts/patch-responsa.py "/path/to/RESPONSA.exe"
```

5. Configure the bottle:

```bash
scripts/configure-wine.sh "/path/to/bottle" "/path/to/soda-11.0-10"
```

6. Install the touch helper:

```bash
scripts/install-touch-helper.sh
```

7. Copy and edit the launcher template:

```bash
install -Dm755 examples/barilan-responsa-launcher.sh "$HOME/.local/bin/barilan-responsa"
```

8. Read [`HANDOFF.md`](HANDOFF.md) before reproducing the full setup on another machine. It documents the complete investigation, exact hashes/offsets, accepted baseline, rejected experiments, fullscreen behavior, DPI findings, and touch-input architecture.

## Verified executable hashes

Original:

```text
7987ad1c1da58f8f4a949451688e987fac44a3c873fe83c1fce4ec5e6a439dcb
```

Patched:

```text
6fcc0381376e5a3a0e57a4b643df08d9a4241141bd8fd3307193d238fede46fe
```

Do not apply the patch to another build without independently re-validating the binary layout.

## Touch scrolling

The application did not expose modern smooth touch scrolling through Wine. The accepted helper:

1. reads XInput2 `TouchBegin` / `TouchUpdate` / `TouchEnd` events;
2. does not grab the input device and does not move the mouse pointer;
3. dynamically locates the largest visible Responsa child HWND with a live vertical scrollbar;
4. sends `WM_VSCROLL` with `SB_LINEUP` / `SB_LINEDOWN`;
5. uses conservative defaults: `PIXELS_PER_STEP=65`, `DEADZONE=6`.

True pixel-smooth kinetic scrolling was not achieved because the application's own MFC scrolling model is discrete.

## Optional drag / long-press mitigation

Mouse tracing identified the content view handling selection and long-press behavior as:

```text
Afx:00400000:82b:00060020:01900020:00000000
```

`src/dragblock-precise.c` subclasses only large visible windows of that exact class. It delays application-visible left-button down until release, replays a normal short click only when the gesture remains within the movement/time thresholds, and suppresses drag/long-hold sequences that would otherwise enter Responsa's text-selection or long-press path.

This is **best-effort, not perfect**. It was accepted as the closest practical behavior reached, not as a complete elimination of every accidental selection or long-press edge case. See [`docs/accepted-state-2026-09-17.md`](docs/accepted-state-2026-09-17.md).

For persistent local use, compile the DLL and use `scripts/inject-dragblock.sh` from the launcher so it waits for `RESPONSA.exe` and injects the filter for each fresh process.

## Non-goals / known rejected paths

- Native Wine Wayland looked sharper but had pointer-coordinate and fullscreen problems in the tested environment.
- `evdev` device grabbing plus `ydotool` was rejected because it interfered with touch and pointer position.
- Full touchscreen ownership with `evdev.grab()` was rejected because it disabled touch for the rest of the desktop while active.
- `SB_THUMBPOSITION` did not move the application's content.
- overlay-based pixel dragging was visually unstable under the tested XWayland scaling chain.
- popup-menu reset/gating and aggressive synthetic mouse-up cleanup were rejected because they could interfere with menu/mouse behavior.

See `HANDOFF.md` for full details.

## License

Original helper code and documentation in this repository are MIT licensed. Third-party/commercial software is not covered by this license and is not distributed here.
