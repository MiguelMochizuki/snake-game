"""
test_clear_level.py
Author: Miguel Mochizuki Silva
Description: E2E: C on the leaderboard clears only the level being shown and stays on it
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
		game.wait_for(r"<Normal>")
		game.wait_for(r"1\.\s+0")
		# Hard has no games: clearing it must leave Normal alone and stay on Hard
		game.send_now("d")
		game.wait_for(r"<Hard>")
		game.send_now("c")
		game.wait_for("CLEAR SCORES")
		game.wait_for(r"score of Hard\?")
		game.send_now("\r")
		game.wait_for(r"<Hard>")
		game.wait_for("No games yet")
		game.send_now("a")
		game.wait_for(r"<Normal>")
		game.wait_for(r"1\.\s+0")
		# Now clear Normal itself
		game.send_now("c")
		game.wait_for(r"score of Normal\?")
		game.send_now("\r")
		game.wait_for(r"<Normal>")
		game.wait_for("No games yet")
		game.send_now("\r")
		game.wait_for(r"> Leaderboard")
		game.send_now("q")
		code = game.exit_code()
		assert code == 0, "exit code %d" % code
