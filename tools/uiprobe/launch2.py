"""Launch with both click and paint debug enabled."""
import os
import subprocess
import sys

env = dict(os.environ)
env['QTCLONE_CLICK_DEBUG'] = '1'
env['QTC_PAINT_DEBUG'] = '1'
p = subprocess.Popen([sys.argv[1]] + sys.argv[2:], env=env, close_fds=True,
                     creationflags=0x00000008)
print(p.pid)
