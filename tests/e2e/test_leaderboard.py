"""
test_leaderboard.py
Author: Miguel Mochizuki Silva
Description: E2E: the leaderboard lists the scores of a level, switches level with D and returns to the menu
"""
import sys
import tempfile

from harness import Game, play_once

MENU = {"SNAKE_SKIP_MENU": ""}

with tempfile.TemporaryDirectory() as tmp:
	play_once(sys.argv[1], tmp)
	with Game(sys.argv[1], cwd=tmp, env=MENU) as game:
		game.wait_for(r"> Start")
		game.send_now(b"\x1b[B\x1b[B\r")
		game.wait_for("TOP SCORES")
		game.wait_for(r"<Normal>")
		game.wait_for(r"1\.\s+0")
		game.send_now("d")
		game.wait_for(r"<Hard>")
		game.wait_for("No games yet")
		game.send_now("\r")
		game.wait_for(r"> Leaderboard")
		game.send_now("q")
		code = game.exit_code()
		assert code == 0, "exit code %d" % code
