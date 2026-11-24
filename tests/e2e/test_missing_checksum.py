"""
test_missing_checksum.py
Author: Miguel Mochizuki Silva
Description: E2E: a missing checksum file zeroes the scores but keeps the play count, and the old file is copied to scores_v2.txt.bak
"""
import os
import re
import sys
import tempfile

from harness import play_once

with tempfile.TemporaryDirectory() as tmp:
	play_once(sys.argv[1], tmp)
	os.remove(os.path.join(tmp, "data", "scores_v2.checksum"))
	panel = play_once(sys.argv[1], tmp)
	assert re.search(r"Games Played:\s+2\n", panel), panel
	assert re.search(r"Best Score:\s+0\n", panel), panel
	with open(os.path.join(tmp, "data", "scores_v2.txt.bak")) as f:
		assert f.read() == "N 0\n"
