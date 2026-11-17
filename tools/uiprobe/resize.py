"""Resize the main window by 1px and back to force a full WM_PAINT."""
import ctypes
import ctypes.wintypes as wt
import sys
import time

pid = int(sys.argv[1])
u = ctypes.windll.user32
found = []


@ctypes.WINFUNCTYPE(ctypes.c_bool, wt.HWND, wt.LPARAM)
def cb(hwnd, lparam):
    p = wt.DWORD()
    u.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
    if p.value == pid:
        cls = ctypes.create_unicode_buffer(128)
        u.GetClassNameW(hwnd, cls, 128)
        if cls.value == 'QtCloneWindow' and u.IsWindowVisible(hwnd):
            r = wt.RECT()
            u.GetWindowRect(hwnd, ctypes.byref(r))
            found.append((hwnd, r.right - r.left, r.bottom - r.top))
    return True


u.EnumWindows(cb, 0)
found.sort(key=lambda x: x[1] * x[2], reverse=True)
hwnd, w, h = found[0]
u.SetWindowPos(hwnd, 0, 0, 0, w + 1, h + 1, 0x0002 | 0x0004)  # NOMOVE|NOZORDER
time.sleep(0.3)
u.SetWindowPos(hwnd, 0, 0, 0, w, h, 0x0002 | 0x0004)
time.sleep(0.4)
print('resized', w, h)
