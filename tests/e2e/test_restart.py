"""
test_restart.py
Author: Miguel Mochizuki Silva
Description: E2E: r on the game-over panel starts a new game that counts as the second one
"""
import re
import sys
import tempfile

from harness import Game, quit_game

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp) as game:
	game.wait_for(r"GAME OVER!")
	game.send("r")
	game.wait_for(r"Score: 0")
	panel = quit_game(game)
	assert re.search(r"Games Played:\s+2\n", panel), panel
