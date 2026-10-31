"""
test_migration.py
Author: Miguel Mochizuki Silva
Description: E2E: valid v1 score files are imported as Normal scores and left untouched
"""
import os
import re
import sys
import tempfile

from harness import Game, quit_game

MASK = (1 << 64) - 1
SEED = 0x9E3779B97F4A7C15


def v1_checksum(best, history):
	"""
	Computes the checksum the v1 releases wrote

	Args:
		best (int): Best score
		history (list[int]): Scores of every game

	Returns:
		int: The 64-bit checksum
	"""
	h = SEED
	for value in [best] + history:
		h ^= (value + SEED + ((h << 6) & MASK) + (h >> 2)) & MASK
	return h


with tempfile.TemporaryDirectory() as tmp:
	data = os.path.join(tmp, "data")
	os.mkdir(data)
	files = {
		"score_history.txt": "3\n8\n",
		"best_score.txt": "8\n",
		"score_checksum.txt": "%d\n" % v1_checksum(8, [3, 8]),
	}
	for name, text in files.items():
		with open(os.path.join(data, name), "w") as f:
			f.write(text)
	with Game(sys.argv[1], cwd=tmp) as game:
		game.wait_for(r"Score: 0")
		panel = quit_game(game)
	assert re.search(r"Level:\s+Normal\n", panel), panel
	assert re.search(r"Best Score:\s+8\n", panel), panel
	assert re.search(r"Games Played:\s+3\n", panel), panel
	for name, text in files.items():
		with open(os.path.join(data, name)) as f:
			assert f.read() == text, name + " changed"
