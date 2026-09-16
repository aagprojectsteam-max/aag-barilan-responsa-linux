# Full Technical Handoff

This repository contains the complete reproduction and troubleshooting handoff for running the verified Bar-Ilan Responsa 25 build on Ubuntu through Bottles/Wine.

The handoff is split into four contiguous parts so it remains easy to review and hand to another AI without losing any of the original detail:

1. [`handoff/part-01.md`](handoff/part-01.md) — architecture, host prerequisites, Bottles setup, application installation, runtime dependencies, critical 2-byte startup patch, locale, and beginning of the fullscreen fix.
2. [`handoff/part-02.md`](handoff/part-02.md) — fullscreen fix completion, XWayland/DPI sharpness work, native Wayland rejection, renderer, Alt+Tab ghost handling, touchscreen architecture, internal MFC scroll-window discovery, and start of the final touch bridge.
3. [`handoff/part-03.md`](handoff/part-03.md) — complete touch bridge/helper implementation, launcher, desktop entry, and verification checklist.
4. [`handoff/part-04.md`](handoff/part-04.md) — final acceptance state, rejected experiments, diagnostics, fresh-machine deployment order, publication guidance, production baseline snapshot, limitations, and explicit instructions for another AI.

## Instruction for another AI

Read all four parts in order before changing anything. Treat the documented accepted production baseline as authoritative. Preserve backups, verify hashes before applying the executable patch, keep the accepted XWayland/DPI/fullscreen configuration unless intentionally re-testing a newer Wine stack, and do not redistribute proprietary application binaries or content.

## Accepted baseline

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
