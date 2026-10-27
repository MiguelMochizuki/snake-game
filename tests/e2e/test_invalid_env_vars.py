"""
test_invalid_env_vars.py
Author: Miguel Mochizuki Silva
Description: E2E: malformed SNAKE_FRAME_MS / SNAKE_SEED fall back to defaults instead of breaking the game
"""
import sys
import tempfile

from harness import Game, quit_game

CASES = [{"SNAKE_FRAME_MS": "abc"}, {"SNAKE_FRAME_MS": "-5"}, {"SNAKE_FRAME_MS": ""}, {"SNAKE_SEED": "xyz"}]

for env in CASES:
	with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=env) as game:
		try:
			game.wait_for(r"Score: 0")
			quit_game(game)
		except AssertionError as e:
			raise AssertionError("%r: %s" % (env, e))
