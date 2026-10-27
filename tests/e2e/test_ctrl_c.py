"""
test_ctrl_c.py
Author: Miguel Mochizuki Silva
Description: E2E: Ctrl+C (byte 0x03 in raw mode) acts as q: it ends the round with the score saved, then exits 0
"""
import os
import sys
import tempfile
import termios

from harness import Game


def history_lines(tmp):
	"""
	Parameters:
	str tmp: The game's working directory

	Returns int: Number of lines in data/score_history.txt
	"""
	with open(os.path.join(tmp, "data", "score_history.txt")) as f:
		return len(f.read().splitlines())


# A fresh pty has the attributes the game's pty had before launch
_master, _slave = os.openpty()
before = termios.tcgetattr(_slave)
os.close(_master)
os.close(_slave)

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=100) as game:
	game.send(b"\x03")
	game.wait_for("GAME OVER!")
	assert history_lines(tmp) == 1
	game.send(b"\x03")
	code = game.exit_code()
	assert code == 0, "exit code %d" % code
	assert game.tty_attrs() == before, "terminal not restored"
	assert history_lines(tmp) == 1
