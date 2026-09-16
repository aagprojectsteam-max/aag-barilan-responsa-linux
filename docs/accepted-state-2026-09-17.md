# Accepted state — 2026-09-17

This document records the locally accepted state after the touch/drag investigation. It is intentionally conservative: the result is not perfect, but it was the closest stable behavior reached and was explicitly chosen as the stopping point.

## Accepted runtime baseline

- Ubuntu Desktop 26.04 LTS / GNOME Wayland.
- Bottles 67.3.
- Soda 11.0-10 / Wine 11.0.
- XWayland path retained.
- Wine DPI 192.
- GDI renderer.
- `GrabFullscreen=N` and `GrabPointer=N`.
- Existing 2-byte startup patch remains required for the verified executable build.
- Touch scrolling remains implemented by the existing helper and Win32 `WM_VSCROLL` bridge.

## Additional accepted experiment: precise content-view drag filter

The closest accepted text-drag/long-press mitigation is `src/dragblock-precise.c`.

Mouse tracing established that the content view which receives the relevant sequence is the MFC class:

```text
Afx:00400000:82b:00060020:01900020:00000000
```

The observed path included:

```text
WM_LBUTTONDOWN
WM_MOUSEMOVE with MK_LBUTTON
WM_LBUTTONUP
WM_TIMER (id 1 during the long-press path)
```

The precise filter therefore subclasses only large visible windows of that exact class. It delays the application-visible left-button down until release, then replays a normal short click only if the pointer did not move beyond the drag threshold and the press duration stayed below the click timeout. Drag/long-hold sequences are swallowed rather than forwarded into Responsa's selection/long-press path.

## Important limitation

This filter is **best-effort, not perfect**. The user accepted it as the closest practical state reached, not as a complete fix. Do not describe it as eliminating every accidental selection, long-press popup, or touch-edge case.

## Persistence model

For a persistent local installation:

1. Compile `src/dragblock-precise.c` as a 32-bit DLL with MinGW.
2. Keep the injector and DLL under a stable local directory such as:

   ```text
   ~/.local/share/barilan-responsa/dragblock/
   ```

3. Start `scripts/inject-dragblock.sh` in the background from the main launcher before launching Responsa. The helper waits for `RESPONSA.exe` and injects the DLL once the process appears.
4. Keep the touch-scroll helper and Alt+Tab helper independent.

## Rejected approaches during this round

The following were explicitly rejected and should not be reintroduced casually:

- `evdev.grab()` / full touchscreen ownership: it disabled touch for the rest of the desktop while active.
- global `SendInput` recreation of tap input.
- popup-menu reset / gating around `#32768`: it could interfere with mouse interaction and menus.
- aggressive `WM_CANCELMODE` / synthetic `WM_LBUTTONUP` cleanup across views.
- overlay-based direct-manipulation scrolling.

## Publication guidance

The repository must not contain the commercial application, installer, database/content files, Microsoft runtime DLLs, or a patched proprietary executable. Publish only helper code, patch logic, documentation, hashes, and reproducible setup instructions.
