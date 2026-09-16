# Changelog

## 1.1.0 - 2026-09-17

- Recorded the user-accepted stopping point for the touch/drag investigation.
- Added `src/dragblock-precise.c`, a best-effort filter for the traced MFC content view class `Afx:00400000:82b:00060020:01900020:00000000`.
- Added persistent injection helper `scripts/inject-dragblock.sh` for fresh Responsa processes.
- Documented that the drag/long-press mitigation is intentionally best-effort and not a complete fix.
- Documented rejected approaches from the final investigation round, including full touchscreen `evdev.grab()` ownership, global tap recreation, popup-menu gating/reset, aggressive synthetic mouse-up cleanup, and overlay-based direct manipulation.
- Added `docs/accepted-state-2026-09-17.md` with the accepted runtime state and persistence model.

## 1.0.0 - 2026-09-16

- Documented the accepted Ubuntu/Bottles/Soda 11.0 architecture.
- Added version-locked `RESPONSA.exe` startup patcher with SHA256 and byte guards.
- Added Wine configuration helper for DPI 192, GDI, and fullscreen/pointer grab settings.
- Added Win32 `WM_VSCROLL` bridge and XInput2 touch helper.
- Added launcher and desktop-entry examples.
- Added full engineering handoff with rejected experiments and reproduction checklist.
