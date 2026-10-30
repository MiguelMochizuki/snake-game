"""
test_persistence.py
Author: Miguel Mochizuki Silva
Description: E2E: two runs in the same cwd append one line per game to scores_v2.txt and write the checksum
"""
import os
import sys
import tempfile

from harness import play_once

with tempfile.TemporaryDirectory() as tmp:
	play_once(sys.argv[1], tmp)
	play_once(sys.argv[1], tmp)
	data = os.path.join(tmp, "data")
	with open(os.path.join(data, "scores_v2.txt")) as f:
		lines = f.read().splitlines()
	assert len(lines) == 2, lines
	assert all(line.startswith("N ") for line in lines), lines
	assert os.path.isfile(os.path.join(data, "scores_v2.checksum"))
