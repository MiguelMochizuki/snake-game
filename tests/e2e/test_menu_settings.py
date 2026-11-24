"""
test_menu_settings.py
Author: Miguel Mochizuki Silva
Description: E2E: the level and sound chosen in the menu are remembered on the next launch; practice mode is not
"""
import os
import sys
import tempfile

from harness import Game, strip_ansi

MENU = {"SNAKE_SKIP_MENU": ""}

with tempfile.TemporaryDirectory() as tmp:
	with Game(sys.argv[1], cwd=tmp, env=MENU) as game:
		game.wait_for(r"> Start")
		# Level row: next level (Hard); down to the Sound row (three rows below): toggle; down again: practice on; quit
		game.send_now("s")
		game.send_now("d")
		game.wait_for(r"Level <Hard>")
		for _ in range(3):
			game.send_now("s")
		game.send_now("a")
		game.wait_for(r"Sound <Off>")
		game.send_now("s")
		game.send_now("a")
		game.wait_for(r"Practice <On>")
		game.send_now("q")
		assert game.exit_code() == 0
	with open(os.path.join(tmp, "data", "settings.txt")) as f:
		assert f.read() == "level=Hard\nsound=off\n"
	with Game(sys.argv[1], cwd=tmp, env=MENU) as game:
		game.wait_for(r"Level <Hard>")
		game.wait_for(r"Sound <Off>")
		game.wait_for(r"Practice <Off>")
		game.send_now("q")
		assert game.exit_code() == 0
