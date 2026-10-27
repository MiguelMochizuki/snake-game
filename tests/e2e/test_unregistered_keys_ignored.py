"""
test_unregistered_keys_ignored.py
Author: Miguel Mochizuki Silva
Description: E2E: keys that are not registered (F-keys, Alt/Ctrl combos, Ctrl+Z, UTF-8, ...) have no effect and are not echoed
"""
import sys
import tempfile
import termios

from harness import Game, UNREGISTERED_KEYS, quit_game, strip_ansi


def head_x(frame):
	"""
	Parameters:
	list[str] frame: Board rows

	Returns int: Rightmost 'O' column on the start row (the head while moving right)
	"""
	return frame[10].rindex("O")


# 100 ms frames: the head starts at x=10 and the wall is 10 moves away, so the whole battery
# fits well before the wall even on a slow reader
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=100) as game:
	game.wait_frames(2)
	mark = len(game.out)
	game.send(b"".join(UNREGISTERED_KEYS))
	game.wait_input_consumed()
	game.drain()
	n = game.frame_count()
	game.wait_frames(n + 2)
	assert "GAME OVER!" not in game.out
	assert game.proc.poll() is None, "process died"
	frames = game.frames()
	assert head_x(frames[-1]) == head_x(frames[-2]) + 1, "snake did not keep moving right"
	assert game.tty_attrs()[3] & termios.ECHO == 0, "ECHO turned on"
	# The renderer's own escapes are stripped; an echoed tail like "[15~" or "OP" would survive
	text = strip_ansi(game.out[mark:])
	for tail in ("OP", "[15~", "[24~", "[5~", "[6~", "[2~", "[3~", "1;5A", "\xc3\xa9"):
		assert tail not in text, "%r was echoed" % tail
	quit_game(game)
