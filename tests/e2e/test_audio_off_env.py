"""
test_audio_off_env.py
Author: Miguel Mochizuki Silva
Description: E2E: with SNAKE_AUDIO=off the bell still rings before the game-over panel, even in a build with audio
"""
import sys
import tempfile

from harness import Game

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env={"SNAKE_AUDIO": "off"}) as game:
	game.wait_for("GAME OVER!")
	game.wait_for("Quit")
	bell = game.out.find("\a")
	assert bell != -1, "no bell with SNAKE_AUDIO=off"
	assert bell < game.out.find("GAME OVER!"), "bell came after the panel"
	game.send("q")
	assert game.exit_code() == 0
