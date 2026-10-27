"""
test_start_and_quit.py
Author: Miguel Mochizuki Silva
Description: E2E: the game starts, q opens the game-over panel, q exits with status 0
"""
import sys
import tempfile

from harness import Game, quit_game

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp) as game:
	game.wait_for(r"Score: 0")
	quit_game(game)
