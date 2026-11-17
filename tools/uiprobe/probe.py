"""Probe QtClone apps from the outside with Win32 only.

Usage:
    python probe.py launch <exe> [args...]      -> prints pid
    python probe.py wins <pid>                  -> visible windows + rects
    python probe.py click <pid> <x> <y> [--double|--right]
    python probe.py type <pid> <text>
    python probe.py close <pid>
    python probe.py capture <out.png> [w] [h]

All coordinates are client coordinates of the largest visible window.
"""
import ctypes
import ctypes.wintypes as wt
import os
import subprocess
import sys
import time

u = ctypes.windll.user32


def qtc_windows(pid):
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
                title = ctypes.create_unicode_buffer(256)
                u.GetWindowTextW(hwnd, title, 256)
                found.append((hwnd, (r.right - r.left, r.bottom - r.top), title.value,
                              (r.left, r.top)))
        return True

    u.EnumWindows(cb, 0)
    return found


def main_window(pid):
    wins = qtc_windows(pid)
    wins.sort(key=lambda w: w[1][0] * w[1][1], reverse=True)
    return wins[0][0] if wins else 0


def click(pid, cx, cy, double=False, right=False):
    hwnd = main_window(pid)
    if not hwnd:
        raise SystemExit('no window')
    pt = wt.POINT(cx, cy)
    u.ClientToScreen(hwnd, ctypes.byref(pt))
    u.SetCursorPos(pt.x, pt.y)
    time.sleep(0.15)
    if right:
        u.mouse_event(0x0008, 0, 0, 0, 0)
        time.sleep(0.05)
        u.mouse_event(0x0010, 0, 0, 0, 0)
    else:
        u.mouse_event(0x0002, 0, 0, 0, 0)
        time.sleep(0.05)
        u.mouse_event(0x0004, 0, 0, 0, 0)
        if double:
            time.sleep(0.06)
            u.mouse_event(0x0002, 0, 0, 0, 0)
            time.sleep(0.05)
            u.mouse_event(0x0004, 0, 0, 0, 0)
    time.sleep(0.4)


def type_text(pid, text):
    hwnd = main_window(pid)
    if not hwnd:
        raise SystemExit('no window')
    for ch in text:
        u.PostMessageW(hwnd, 0x0102, ord(ch), 0)
        time.sleep(0.03)
    time.sleep(0.3)


def send_close(pid):
    hwnd = main_window(pid)
    if not hwnd:
        raise SystemExit('no window')
    u.PostMessageW(hwnd, 0x0010, 0, 0)
    time.sleep(0.5)


def capture(path, width=1600, height=1000):
    from PIL import Image
    gdi32 = ctypes.windll.gdi32
    screen = u.GetDC(0)
    mem = gdi32.CreateCompatibleDC(screen)
    bmp = gdi32.CreateCompatibleBitmap(screen, width, height)
    gdi32.SelectObject(mem, bmp)
    gdi32.BitBlt(mem, 0, 0, width, height, screen, 0, 0, 0x00CC0020)

    class BIH(ctypes.Structure):
        _fields_ = [('biSize', wt.DWORD), ('biWidth', ctypes.c_long), ('biHeight', ctypes.c_long),
                    ('biPlanes', wt.WORD), ('biBitCount', wt.WORD), ('biCompression', wt.DWORD),
                    ('biSizeImage', wt.DWORD), ('biXPelsPerMeter', ctypes.c_long),
                    ('biYPelsPerMeter', ctypes.c_long), ('biClrUsed', wt.DWORD),
                    ('biClrImportant', wt.DWORD)]

    bi = BIH()
    bi.biSize = ctypes.sizeof(BIH)
    bi.biWidth = width
    bi.biHeight = -height
    bi.biPlanes = 1
    bi.biBitCount = 32
    bi.biCompression = 0
    buf = ctypes.create_string_buffer(width * height * 4)
    gdi32.GetDIBits(mem, bmp, 0, height, buf, ctypes.byref(bi), 0)
    Image.frombuffer('RGB', (width, height), buf, 'raw', 'BGRX', 0, 1).save(path)
    gdi32.DeleteObject(bmp)
    gdi32.DeleteDC(mem)
    u.ReleaseDC(0, screen)
    return path


def launch(exe, args):
    env = dict(os.environ)
    env['QTCLONE_CLICK_DEBUG'] = '1'
    p = subprocess.Popen([exe] + args, env=env, close_fds=True,
                         creationflags=0x00000008)
    return p.pid


def main():
    cmd = sys.argv[1]
    if cmd == 'launch':
        print(launch(sys.argv[2], sys.argv[3:]))
    elif cmd == 'wins':
        for hwnd, size, title, pos in qtc_windows(int(sys.argv[2])):
            print('hwnd', hex(hwnd), 'size', size, 'pos', pos, repr(title))
    elif cmd == 'click':
        click(int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]),
              double='--double' in sys.argv, right='--right' in sys.argv)
    elif cmd == 'type':
        type_text(int(sys.argv[2]), sys.argv[3])
    elif cmd == 'close':
        send_close(int(sys.argv[2]))
    elif cmd == 'capture':
        w = int(sys.argv[3]) if len(sys.argv) > 3 else 1600
        h = int(sys.argv[4]) if len(sys.argv) > 4 else 1000
        print(capture(sys.argv[2], w, h))
    else:
        raise SystemExit('unknown command ' + cmd)


if __name__ == '__main__':
    main()
