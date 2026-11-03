"""
test_gui_smoke.py
Author: Miguel Mochizuki Silva
Description: E2E: snake-gui starts on SDL's dummy video driver, keeps running, and exits with status 0 when asked to close
"""
import os
import signal
import subprocess
import sys
import tempfile
import time

with tempfile.TemporaryDirectory() as tmp:
	env = dict(os.environ, SNAKE_SKIP_MENU="1", SDL_VIDEODRIVER="dummy", SNAKE_AUDIO="off", SNAKE_SEED="1")
	proc = subprocess.Popen([sys.argv[1]], cwd=tmp, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
	time.sleep(2)
	assert proc.poll() is None, "snake-gui exited early: " + proc.stderr.read().decode()
	proc.send_signal(signal.SIGTERM)  # SDL turns it into a quit event, like the window close button
	try:
		proc.wait(timeout=5)
	except subprocess.TimeoutExpired:
		proc.kill()
		raise AssertionError("snake-gui did not exit after the close request")
	assert proc.returncode == 0, "exit status %d" % proc.returncode
