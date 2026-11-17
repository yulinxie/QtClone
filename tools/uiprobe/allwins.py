"""List all windows (visible or not) for a pid."""
import ctypes
import ctypes.wintypes as wt
import sys

pid = int(sys.argv[1])
u = ctypes.windll.user32
rows = []


@ctypes.WINFUNCTYPE(ctypes.c_bool, wt.HWND, wt.LPARAM)
def cb(hwnd, lparam):
    p = wt.DWORD()
    u.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
    if p.value == pid:
        cls = ctypes.create_unicode_buffer(128)
        u.GetClassNameW(hwnd, cls, 128)
        title = ctypes.create_unicode_buffer(256)
        u.GetWindowTextW(hwnd, title, 256)
        r = wt.RECT()
        u.GetWindowRect(hwnd, ctypes.byref(r))
        rows.append((hex(hwnd), cls.value, u.IsWindowVisible(hwnd),
                     (r.left, r.top, r.right - r.left, r.bottom - r.top),
                     title.value))
    return True


u.EnumWindows(cb, 0)
for row in rows:
    print(row)
