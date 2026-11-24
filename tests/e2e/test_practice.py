"""
test_practice.py
Author: Miguel Mochizuki Silva
Description: E2E: a practice game says so on the status line and on the end panel, shows no games count and saves nothing
"""
import os
import sys
import tempfile

from harness import Game, strip_ansi

MENU = {"SNAKE_SKIP_MENU": ""}

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=MENU) as game:
	game.wait_for(r"> Start")
	# Practice is the sixth row: down five times, switch it on, back up to Start and begin
	for _ in range(5):
		game.send_now("s")
	game.send_now("a")
	game.wait_for(r"Practice <On>")
	for _ in range(5):
		game.send_now("w")
	game.send_now("\r")
	game.wait_for(r"Best: 0  PRACTICE  P pause")
	game.wait_for(r"GAME OVER!")
	game.wait_for(r"Session Best:\s+0")
	game.wait_for(r"PRACTICE - not saved")
	assert "Games Played" not in strip_ansi(game.out[game.out.rfind("GAME OVER!"):])
	game.send("q")
	assert game.exit_code() == 0
	saved = os.path.join(tmp, "data", "scores_v2.txt")
	assert not os.path.exists(saved) or open(saved).read() == "", "a practice game was saved"
