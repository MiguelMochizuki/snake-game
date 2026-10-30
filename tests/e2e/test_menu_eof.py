"""
test_menu_eof.py
Author: Miguel Mochizuki Silva
Description: E2E: with stdin at EOF the start menu exits 0 without starting a game
"""
import os
import subprocess
import sys
import tempfile

from harness import scale

env = dict(os.environ, SNAKE_SKIP_MENU="")
with tempfile.TemporaryDirectory() as tmp:
	p = subprocess.Popen([os.path.abspath(sys.argv[1])], cwd=tmp, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE)
	try:
		out, _ = p.communicate(timeout=10 * scale())
	except subprocess.TimeoutExpired:
		p.kill()
		p.communicate()
		raise AssertionError("menu did not exit on stdin EOF")
assert p.returncode == 0, "exit code %r" % p.returncode
assert b"Start" in out, "menu was not shown"
assert b"GAME OVER!" not in out, "a game was started"
