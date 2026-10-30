"""
test_eof_exits.py
Author: Miguel Mochizuki Silva
Description: E2E: with stdin at EOF the game reaches game over and exits 0 instead of spinning
"""
import os
import subprocess
import sys
import tempfile

from harness import scale

env = dict(os.environ, SNAKE_FRAME_MS=str(int(30 * scale())), SNAKE_SEED="42", SNAKE_SKIP_MENU="1")
with tempfile.TemporaryDirectory() as tmp:
	p = subprocess.Popen([os.path.abspath(sys.argv[1])], cwd=tmp, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE)
	try:
		out, _ = p.communicate(timeout=10 * scale())
	except subprocess.TimeoutExpired:
		p.kill()
		p.communicate()
		raise AssertionError("game did not exit on stdin EOF")
assert p.returncode == 0, "exit code %r" % p.returncode
assert b"GAME OVER!" in out, "no game over screen in output"
