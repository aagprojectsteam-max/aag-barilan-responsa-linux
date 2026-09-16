# Accepted state — 2026-09-17

This document records the final locally accepted production state after the Wine/touch investigation. The result is intentionally conservative: the working configuration is preserved, known limitations are documented, and rejected experiments are not part of production.

## Accepted runtime baseline

- Ubuntu Desktop 26.04 LTS / GNOME Wayland.
- Bottles 67.3.
- Soda 11.0-10 / Wine 11.0.
- XWayland path retained.
- Wine DPI 192.
- GDI renderer.
- `GrabFullscreen=N` and `GrabPointer=N`.
- Existing 2-byte startup patch remains required for the verified executable build.
- Touch scrolling is provided by the accepted XInput2 helper and Win32 `WM_VSCROLL` bridge.
- Main-window tap works in the accepted state.
- Tap activation inside Win32 popup menus (`#32768`) remains a known limitation; mouse activation still works.

## Precise content-view drag filter

The closest accepted text-drag/long-press mitigation is `src/dragblock-precise.c`.

Mouse tracing established that the content view receiving the relevant sequence is the MFC class:

```text
Afx:00400000:82b:00060020:01900020:00000000
```

The observed path included `WM_LBUTTONDOWN`, `WM_MOUSEMOVE` with `MK_LBUTTON`, `WM_LBUTTONUP`, and `WM_TIMER` id 1 during the long-press path. The filter subclasses only large visible windows of that exact class, delays application-visible left-button down until release, replays a normal short click only inside the movement/time thresholds, and suppresses drag/long-hold sequences.

This mitigation is **best-effort, not perfect**. It must not be described as eliminating every accidental selection, long-press popup, or touch edge case.

## Rejected popup-menu experiments

Several approaches were tested and explicitly rejected because they either failed to activate menu items reliably or interfered with normal tap behavior:

- synthetic Win32 `WM_LBUTTONDOWN` / `WM_LBUTTONUP` into `#32768`;
- `xdotool` / XTest click recreation;
- direct `WM_COMMAND` attempts after probing menu state;
- keyboard navigation / synthetic `Home` + `Down` + `Enter` activation;
- running an additional XInput2 listener alongside the accepted touch helper;
- popup reset/gating and aggressive synthetic mouse-up cleanup.

The final production helper therefore does **not** include popup-tap emulation.

## Wine touch/gesture experiment

Wine merge request 11663 (`win32u: Implement touch and gesture input support`, commit `a946158939554cbec2c90aab83808d241b7579fa`) was fetched and source-verified. A separate experimental build was started and intentionally stopped before promotion. No experimental runner replaced Soda 11.0 in production.

Future testing of newer Wine touch support should always use an isolated runner and a disposable/backup prefix first.

## Local storage cleanup

The installed bottle contained a second ~11 GiB `drive_c/BarIlan-Installer` tree. Before removal, the running application had no open files from that tree and no configuration references were found in the checked bottle configuration/text files. The installer copy was removed after acceptance testing, reducing the production bottle from roughly 22 GiB to roughly 12 GiB. The installed application under `Program Files (x86)/ResponsaCD25` was preserved and revalidated afterward.

## Persistence model

For a persistent local installation:

1. keep the accepted touch helper and `scrollbridge.exe` together;
2. keep the optional drag filter injector/DLL in a stable local directory;
3. launch the accepted helpers from the main launcher only once per Responsa process;
4. do not start a second `xinput test-xi2 --root` listener for popup experiments;
5. keep the Alt+Tab helper independent;
6. preserve the verified executable hashes and Wine registry/DPI settings in the private DATA checkpoint.

## Publication guidance

The repository must not contain the commercial application, installer, database/content files, Microsoft runtime DLLs, or a patched proprietary executable. Publish only helper code, patch logic, documentation, hashes, and reproducible setup instructions.
