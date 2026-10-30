"""
test_menu_from_game_over.py
Author: Miguel Mochizuki Silva
Description: E2E: m on the game-over panel returns to the menu with fresh stats and the chosen level kept, and a new game starts from it
"""
import sys
import tempfile

from harness import Game

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp) as game:
	game.wait_for("GAME OVER!")
	game.wait_for("Quit")
	game.send("m")
	game.wait_for(r"> Start")
	game.wait_for(r"Games:\s+1 ")
	# Down, Right: the Level row moves from Normal to Hard; Up, Enter: back on Start and play
	game.send_now(b"\x1b[B\x1b[C")
	game.wait_for(r"<Hard>")
	game.send_now(b"\x1b[A\r")
	game.wait_for(r"Score: 0")
	game.wait_for("GAME OVER!")
	game.wait_for("Quit")
	game.send("M")
	game.wait_for(r"<Hard>")
	game.wait_for(r"Games:\s+2 ")
	game.send_now("q")
	code = game.exit_code()
	assert code == 0, "exit code %d" % code
