"""
test_menu_start.py
Author: Miguel Mochizuki Silva
Description: E2E: the start menu shows the stats, and Enter on Start begins the game
"""
import os
import sys
import tempfile

from harness import Game, play_once, quit_game

MENU = {"SNAKE_SKIP_MENU": ""}

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=MENU) as game:
	game.wait_for(r"> Start")
	game.wait_for(r"Best:\s+0 ")
	game.send_now("\r")
	game.wait_for(r"Score: 0")
	quit_game(game)

# Missing score checksum wipes the history: the menu shows zeros instead of crashing
with tempfile.TemporaryDirectory() as tmp:
	play_once(sys.argv[1], tmp)
	os.remove(os.path.join(tmp, "data", "score_checksum.txt"))
	with Game(sys.argv[1], cwd=tmp, env=MENU) as game:
		game.wait_for(r"Best:\s+0 ")
		game.wait_for(r"Games:\s+0 ")
		game.send_now("\r")
		game.wait_for(r"Score: 0")
		quit_game(game)
