"""
test_arrows_steer.py
Author: Miguel Mochizuki Silva
Description: E2E: the arrow keys steer the snake up, left, down and right without ending the game
"""
import sys
import tempfile

from harness import Game, quit_game


def cells(frame):
	"""
	Parameters:
	list[str] frame: Board rows

	Returns set: (x, y) of every snake cell
	"""
	return {(x, y) for y, row in enumerate(frame) for x, c in enumerate(row) if c == "O"}


def steer(game, key, dx, dy):
	"""
	Sends one arrow key and checks the head moves one cell in (dx, dy) on each of the next 2 frames

	Parameters:
	Game game: The running game
	bytes key: Arrow escape sequence
	int dx: Expected column change per frame
	int dy: Expected row change per frame

	Throws AssertionError: If the head does not move that way
	"""
	game.send(key)
	game.wait_input_consumed()
	game.drain()
	game.wait_frames(game.frame_count() + 3)
	frames = game.frames()[-3:]
	for before, after in zip(frames, frames[1:]):
		new = cells(after) - cells(before)
		assert len(new) == 1, "expected one new head cell, got %s" % new
		x, y = new.pop()
		assert (x - dx, y - dy) in cells(before), "head went to %s, expected direction %s" % ((x, y), (dx, dy))


# 100 ms frames leave slack for reader lag; each leg stays well inside the 20x20 board
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=100) as game:
	steer(game, b"\x1b[A", 0, -1)
	steer(game, b"\x1b[D", -1, 0)
	steer(game, b"\x1b[B", 0, 1)
	steer(game, b"\x1b[C", 1, 0)
	assert "GAME OVER!" not in game.out
	quit_game(game)
