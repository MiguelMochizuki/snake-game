"""
test_wall_death.py
Author: Miguel Mochizuki Silva
Description: E2E: with no input the snake hits the right wall and the panel shows score 0, one game
"""
import sys
import tempfile

from harness import Game

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp) as game:
	game.wait_for(r"GAME OVER!")
	game.wait_for(r"Final Score:\s+0\n")
	game.wait_for(r"Games Played:\s+1\n")
	game.send("q")
	assert game.exit_code() == 0
