"""
test_reverse_ignored.py
Author: Miguel Mochizuki Silva
Description: E2E: pressing the opposite direction (a while moving right) is ignored, not fatal
"""
import sys
import tempfile

from harness import Game, quit_game

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp) as game:
	game.wait_frames(2)
	game.send("a")
	# The key lands in frame n+1 or n+2; frame n+3 proves the snake survived it
	game.wait_frames(game.frame_count() + 3)
	assert "GAME OVER!" not in game.out
	quit_game(game)
