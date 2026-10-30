"""
test_menu_quit.py
Author: Miguel Mochizuki Silva
Description: E2E: q and Ctrl+C at the start menu exit 0 without playing and restore the terminal
"""
import os
import sys
import tempfile
import termios

from harness import Game, strip_ansi

MENU = {"SNAKE_SKIP_MENU": ""}

# A fresh pty has the attributes the game's pty had before launch
_master, _slave = os.openpty()
before = termios.tcgetattr(_slave)
os.close(_master)
os.close(_slave)

for key in ("q", b"\x03"):
	with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=MENU) as game:
		game.wait_for(r"> Start")
		game.send_now(key)
		code = game.exit_code()
		assert code == 0, "%r: exit code %d" % (key, code)
		assert "GAME OVER" not in strip_ansi(game.out), "%r: a game was played" % (key,)
		assert game.tty_attrs() == before, "%r: terminal not restored" % (key,)
