"""
test_eating.py
Author: Miguel Mochizuki Silva
Description: E2E: steer the snake onto the first food; score, final score and best score become 1
"""
import re
import sys
import tempfile

from harness import Game, quit_game

KEYS = {"w": (0, -1), "s": (0, 1), "a": (-1, 0), "d": (1, 0)}


def snake_cells(frame):
	"""
	Parameters:
	list[str] frame: Board rows

	Returns set: (x, y) of every 'O'
	"""
	return {(x, y) for y, row in enumerate(frame) for x, ch in enumerate(row) if ch == "O"}


def choose(head, d, food):
	"""
	Greedy step toward the food that never reverses

	Parameters:
	tuple head: Head (x, y)
	tuple d: Current direction (dx, dy)
	tuple food: Food (x, y)

	Returns str: Key for the wanted direction
	"""
	dx, dy = food[0] - head[0], food[1] - head[1]
	wanted = []
	if dx:
		wanted.append("d" if dx > 0 else "a")
	if dy:
		wanted.append("s" if dy > 0 else "w")
	wanted = [k for k in wanted if KEYS[k] != (-d[0], -d[1])]
	for k in wanted:
		if KEYS[k] == d:
			return k
	if wanted:
		return wanted[0]
	# Food straight behind: sidestep toward the board centre
	if d[1] == 0:
		return "s" if head[1] < 10 else "w"
	return "d" if head[0] < 10 else "a"


with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=100, seed=42) as game:
	game.wait_frames(1)
	food = game.find("*")
	head, d = (10, 10), (1, 0)
	prev = snake_cells(game.frames()[0])
	seen = 1
	while head != food:
		assert seen < 200, "snake never reached the food"
		game.wait_frames(seen + 1)
		frames = game.frames()
		# Track the head through every frame (the new 'O' each step) so none is skipped
		for frame in frames[seen:]:
			cur = snake_cells(frame)
			new = cur - prev
			assert len(new) == 1, "cannot track head: %r" % new
			nxt = new.pop()
			d, head, prev = (nxt[0] - head[0], nxt[1] - head[1]), nxt, cur
		seen = len(frames)
		if head != food:
			key = choose(head, d, food)
			if KEYS[key] != d:
				game.send(key)
	game.wait_for(r"Score: 1\n")
	panel = quit_game(game)
	assert re.search(r"Final Score:\s+1\n", panel), panel
	assert re.search(r"Best Score:\s+1\n", panel), panel
