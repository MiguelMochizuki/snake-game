"""
test_missing_checksum.py
Author: Miguel Mochizuki Silva
Description: E2E: a missing checksum file wipes the history
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
	assert re.search(r"Games Played:\s+1\n", panel), panel
