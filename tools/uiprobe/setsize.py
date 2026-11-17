"""Resize the main window to an explicit size and force a repaint."""
import ctypes
import ctypes.wintypes as wt
import sys
import time

pid = int(sys.argv[1])
w = int(sys.argv[2])
h = int(sys.argv[3])
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
            found.append((hwnd, (r.right - r.left) * (r.bottom - r.top)))
    return True


u.EnumWindows(cb, 0)
found.sort(key=lambda x: x[1], reverse=True)
hwnd = found[0][0]
u.SetWindowPos(hwnd, 0, 0, 0, w, h, 0x0002 | 0x0004)  # NOMOVE | NOZORDER
time.sleep(0.4)
print('resized to', w, h)
