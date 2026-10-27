"""
test_no_color.py
Author: Miguel Mochizuki Silva
Description: E2E: NO_COLOR=1 removes every SGR sequence from the output; without it the head is colored
"""
import re
import sys
import tempfile

from harness import Game, quit_game

ALLOWED = ("\033[2J", "\033[H")

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env={"NO_COLOR": "1"}) as game:
	game.wait_for(r"Score: 0")
	quit_game(game)
	extra = [s for s in re.findall(r"\033\[[0-9;]*[A-Za-z]", game.out) if s not in ALLOWED]
	assert not extra, "unexpected escapes with NO_COLOR=1: %r" % extra[:5]

env = {"NO_COLOR": ""}
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=env) as game:
	game.wait_for(r"Score: 0")
	quit_game(game)
	assert "\033[1;92m" in game.out, "no colored head in the default output"
