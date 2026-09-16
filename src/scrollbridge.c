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
                LONG area = w * h;

                if (w > 200 && h > 100 && area > best_area) {
                    best = child;
                    best_area = area;
                }
            }
        }

        scan(child);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

static BOOL CALLBACK enum_top(HWND hwnd, LPARAM lp)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    if (is_responsa_pid(pid))
        scan(hwnd);

    return TRUE;
}

static HWND find_target(void)
{
    best = NULL;
    best_area = 0;
    EnumWindows(enum_top, 0);
    return best;
}

int main(void)
{
    char line[32];

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("SCROLLBRIDGE_READY\n");

    while (fgets(line, sizeof(line), stdin)) {
        HWND hwnd = find_target();

        if (!hwnd) {
            printf("NO_TARGET\n");
            continue;
        }

        if (line[0] == 'D')
            SendMessageW(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
        else if (line[0] == 'U')
            SendMessageW(hwnd, WM_VSCROLL, SB_LINEUP, 0);
        else if (line[0] == 'Q')
            break;
    }

    return 0;
}
