"""
test_terminal_restored.py
Author: Miguel Mochizuki Silva
Description: E2E: the game puts the tty in raw mode while running and restores ICANON and ECHO on exit
"""
import sys
import tempfile
import termios

from harness import Game, quit_game

FLAGS = termios.ICANON | termios.ECHO

with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp) as game:
	game.wait_frames(2)
	assert game.tty_attrs()[3] & FLAGS == 0, "raw mode not active while running"
	quit_game(game)
	assert game.tty_attrs()[3] & FLAGS == FLAGS, "terminal not restored"
