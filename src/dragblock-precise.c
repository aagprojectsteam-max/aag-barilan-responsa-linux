#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#include <stdlib.h>

#define SUBCLASS_ID 0xA027
#define DRAG_THRESHOLD 12
#define CLICK_MAX_MS 500

typedef struct {
    int pending;
    int dragged;
    POINT down;
    DWORD tick;
} STATE;

static volatile LONG running = 1;

static int moved(POINT a, POINT b)
{
    return abs(a.x - b.x) >= DRAG_THRESHOLD ||
           abs(a.y - b.y) >= DRAG_THRESHOLD;
}

static LRESULT CALLBACK FilterProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
    UINT_PTR id, DWORD_PTR ref)
{
    STATE *st = (STATE *)ref;

    switch (msg) {
    case WM_LBUTTONDOWN:
        st->pending = 1;
        st->dragged = 0;
        st->down.x = (short)LOWORD(lp);
        st->down.y = (short)HIWORD(lp);
        st->tick = GetTickCount();
        return 0;

    case WM_MOUSEMOVE:
        if (st->pending && (wp & MK_LBUTTON)) {
            POINT p;
            p.x = (short)LOWORD(lp);
            p.y = (short)HIWORD(lp);
            if (moved(st->down, p)) st->dragged = 1;
            return 0;
        }
        break;

    case WM_LBUTTONUP:
        if (st->pending) {
            POINT up;
            DWORD elapsed;
            up.x = (short)LOWORD(lp);
            up.y = (short)HIWORD(lp);
            elapsed = GetTickCount() - st->tick;

            int click = !st->dragged &&
                        !moved(st->down, up) &&
                        elapsed <= CLICK_MAX_MS;

            st->pending = 0;
            st->dragged = 0;

            if (click) {
                LPARAM down_lp = MAKELPARAM(
                    (short)st->down.x,
                    (short)st->down.y
                );
                DefSubclassProc(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, down_lp);
                return DefSubclassProc(hwnd, WM_LBUTTONUP, 0, lp);
            }
            return 0;
        }
        break;

    case WM_TIMER:
        if (st->pending && wp == 1) return 0;
        break;

    case WM_CANCELMODE:
    case WM_CAPTURECHANGED:
        st->pending = 0;
        st->dragged = 0;
        break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, FilterProc, SUBCLASS_ID);
        free(st);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }

    return DefSubclassProc(hwnd, msg, wp, lp);
}

static int exact_content_view(HWND hwnd)
{
    WCHAR cls[256] = {0};
    RECT r;

    if (!IsWindowVisible(hwnd)) return 0;
    GetClassNameW(hwnd, cls, 255);

    if (wcscmp(
        cls,
        L"Afx:00400000:82b:00060020:01900020:00000000"
    ) != 0) return 0;

    if (!GetWindowRect(hwnd, &r)) return 0;
    if ((r.right-r.left) < 500 || (r.bottom-r.top) < 300) return 0;
    return 1;
}

static void ensure_hook(HWND hwnd)
{
    DWORD_PTR ref = 0;
    if (!exact_content_view(hwnd)) return;

    if (GetWindowSubclass(hwnd, FilterProc, SUBCLASS_ID, &ref)) return;

    STATE *st = calloc(1, sizeof(*st));
    if (!st) return;

    if (!SetWindowSubclass(
        hwnd, FilterProc, SUBCLASS_ID, (DWORD_PTR)st
    )) free(st);
}

static BOOL CALLBACK child_cb(HWND hwnd, LPARAM lp)
{
    ensure_hook(hwnd);
    return TRUE;
}

static BOOL CALLBACK top_cb(HWND hwnd, LPARAM lp)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != GetCurrentProcessId()) return TRUE;

    ensure_hook(hwnd);
    EnumChildWindows(hwnd, child_cb, 0);
    return TRUE;
}

static DWORD WINAPI watcher(LPVOID unused)
{
    Sleep(250);
    while (InterlockedCompareExchange(&running, 1, 1)) {
        EnumWindows(top_cb, 0);
        Sleep(150);
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(inst);
        HANDLE th = CreateThread(NULL, 0, watcher, NULL, 0, NULL);
        if (th) CloseHandle(th);
    }

    if (reason == DLL_PROCESS_DETACH)
        InterlockedExchange(&running, 0);

    return TRUE;
}
