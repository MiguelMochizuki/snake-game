"""
test_persistence.py
Author: Miguel Mochizuki Silva
Description: E2E: two runs in the same cwd append history, keep the best score and write the checksum
"""
import os
import sys
import tempfile

from harness import play_once

with tempfile.TemporaryDirectory() as tmp:
	play_once(sys.argv[1], tmp)
	play_once(sys.argv[1], tmp)
	data = os.path.join(tmp, "data")
	with open(os.path.join(data, "score_history.txt")) as f:
		history = [int(line) for line in f.read().splitlines()]
	with open(os.path.join(data, "best_score.txt")) as f:
		best = int(f.read())
	assert len(history) == 2, history
	assert best == max(history), (best, history)
	assert os.path.isfile(os.path.join(data, "score_checksum.txt"))
