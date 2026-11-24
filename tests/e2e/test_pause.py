"""
test_pause.py
Author: Miguel Mochizuki Silva
Description: E2E: P stops the game and hides the board, P again goes on, a game paused and then lost is saved, and Q while paused ends the game without saving it
"""
import os
import re
import sys
import tempfile
import time

from harness import Game, FRAME_END, strip_ansi

# A slow step leaves time to pause before the snake reaches the wall
SLOW = 250

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=SLOW) as game:
	game.wait_for(FRAME_END)
	game.send("p")
	game.wait_for(r"PAUSED")
	frozen = len(strip_ansi(game.out))
	time.sleep(SLOW * 4 / 1000.0)
	game._pump(0)
	assert "GAME OVER" not in strip_ansi(game.out), "the game went on while paused"
	assert len(strip_ansi(game.out)) == frozen, "something was drawn while paused"
	# Keys typed while paused do not steer the snake after the resume
	game.send("w")
	game.send("p")
	game.wait_for(FRAME_END)
	game.wait_for(r"GAME OVER!")
	game.wait_for(r"Games Played:\s+1\n")
	game.send("q")
	assert game.exit_code() == 0

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=SLOW) as game:
	game.wait_for(FRAME_END)
	game.send("p")
	game.wait_for(r"PAUSED")
	game.send("q")
	game.wait_for(r"GAME OVER!")
	game.wait_for(r"Games Played:\s+0\n")
	game.send("q")
	assert game.exit_code() == 0
