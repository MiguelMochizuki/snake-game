"""
test_reverse_ignored.py
Author: Miguel Mochizuki Silva
Description: E2E: pressing the opposite direction (a while moving right) is ignored, not fatal
"""
import sys
import tempfile

from harness import Game, quit_game


def head_x(frame):
	"""
	Parameters:
	list[str] frame: Board rows

	Returns int: Rightmost 'O' column on the start row (the head while moving right)
	"""
	return frame[10].rindex("O")


# 100 ms frames: the head starts at x=10 and the wall is 10 moves away, so this leaves
# several frames of slack for reader lag (and frame_ms is multiplied by the timeout scale)
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=100) as game:
	game.wait_frames(2)
	game.send("a")
	# Only once the game has read the key do the following frames prove anything
	game.wait_input_consumed()
	game.drain()
	game.wait_frames(game.frame_count() + 2)
	assert "GAME OVER!" not in game.out
	frames = game.frames()
	assert head_x(frames[-1]) == head_x(frames[-2]) + 1, "snake did not keep moving right"
	quit_game(game)
