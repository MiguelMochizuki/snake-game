"""
test_sound.py
Author: Miguel Mochizuki Silva
Description: E2E: the bell rings right before the game-over panel, and not at all with Sound Off in the menu
"""
import sys
import tempfile

from harness import Game

MENU = {"SNAKE_SKIP_MENU": ""}

# Skipped menu: sound is on, so the bell comes before the panel
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp) as game:
	game.wait_for("GAME OVER!")
	game.wait_for("Quit")
	bell = game.out.find("\a")
	assert bell != -1, "no bell"
	assert bell < game.out.find("GAME OVER!"), "bell came after the panel"
	game.send("q")
	assert game.exit_code() == 0

# Sound Off in the menu: four Downs reach Sound, Enter toggles it, four Ups return to Start
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, env=MENU) as game:
	game.wait_for(r"> Start")
	game.send_now(b"\x1b[B" * 4 + b"\r")
	game.wait_for(r"<Off>")
	game.send_now(b"\x1b[A" * 4 + b"\r")
	game.wait_for(r"Score: 0")
	game.wait_for("GAME OVER!")
	game.wait_for("Quit")
	assert "\a" not in game.out, "bell rang with Sound Off"
	game.send("q")
	assert game.exit_code() == 0
