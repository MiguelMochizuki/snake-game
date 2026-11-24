"""
test_small_terminal.py
Author: Miguel Mochizuki Silva
Description: E2E: a terminal smaller than 30x24 gets a notice before the menu, any key goes on and Q quits; 80x24 and an unsized pty get none
"""
import sys
import tempfile

from harness import Game, quit_game

MENU = {"SNAKE_SKIP_MENU": ""}

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=MENU, size=(10, 40)) as game:
	game.wait_for(r"Terminal too small: 40x10")
	game.wait_for(r"30 columns by 24 lines")
	game.send_now("x")
	game.wait_for(r"> Start")
	game.send_now("q")
	assert game.exit_code() == 0

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=MENU, size=(24, 20)) as game:
	game.wait_for(r"Terminal too small: 20x24")
	game.send_now("q")
	assert game.exit_code() == 0

for size in ((24, 80), None):
	with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=MENU, size=size) as game:
		game.wait_for(r"> Start")
		assert "too small" not in game.out
		game.send_now("q")
		assert game.exit_code() == 0
