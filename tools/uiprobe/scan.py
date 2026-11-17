"""Scan click targets down a column to locate a widget band."""
import ctypes
import ctypes.wintypes as wt
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import probe  # noqa: E402

pid = int(sys.argv[1])
x = int(sys.argv[2])
ys = [int(v) for v in sys.argv[3:]]
log = os.path.join(os.environ.get('TEMP', '.'), 'qtclone-clicks.log')

for y in ys:
    before = 0
    try:
        with open(log, 'a+', encoding='utf-8', errors='replace') as f:
            f.seek(0)
            before = len(f.read())
    except OSError:
        pass
    probe.click(pid, x, y)
    time.sleep(0.3)
    text = ''
    try:
        with open(log, 'r', encoding='utf-8', errors='replace') as f:
            f.seek(before)
            text = f.read().strip()
    except OSError:
        pass
    print('y=%d -> %s' % (y, text.replace('\n', ' | ') or '(no log)'))
