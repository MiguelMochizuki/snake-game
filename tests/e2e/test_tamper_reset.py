"""
test_tamper_reset.py
Author: Miguel Mochizuki Silva
Description: E2E: an edited history fails the checksum, scores reset to zero but the play count is kept
"""
import os
import re
import sys
import tempfile

from harness import play_once

with tempfile.TemporaryDirectory() as tmp:
	play_once(sys.argv[1], tmp)
	with open(os.path.join(tmp, "data", "score_history.txt"), "w") as f:
		f.write("999\n")
	panel = play_once(sys.argv[1], tmp)
	assert re.search(r"Best Score:\s+0\n", panel), panel
	assert re.search(r"Games Played:\s+2\n", panel), panel
