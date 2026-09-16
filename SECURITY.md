# Security

## Scope

This repository contains documentation and helper tooling only. It does not distribute the commercial application or its data.

## Binary patch safety

`scripts/patch-responsa.py` is intentionally fail-closed. It checks:

- the full original SHA256;
- the exact expected two bytes at the known offset;
- the full expected post-patch SHA256.

Do not remove these checks when adapting the script to another application build.

## Input helper safety

The accepted touch helper does not use `EVIOCGRAB`, does not write to `/dev/input`, does not use `ydotool`, and does not move the pointer. It observes XInput2 events and sends `WM_VSCROLL` only to a dynamically selected window owned by `RESPONSA.exe`.

## Reporting

When reporting a problem, include Ubuntu/GNOME version, Bottles version, Wine/Soda runner version, monitor scale, XWayland geometry, executable SHA256, and whether the issue reproduces without the touch helper.
