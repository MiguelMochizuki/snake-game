"""
test_gameover_ignores_keys.py
Author: Miguel Mochizuki Silva
Description: E2E: on the game-over panel only r and q act; an r typed just before death does not restart
"""
import os
import sys
import tempfile

from harness import Game, UNREGISTERED_KEYS, quit_game, strip_ansi

# 100 ms frames; the head starts at x=10 and dies on the move past x=19
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=100) as game:
	game.wait_frames(2)
	# One r typed while the snake is still alive: it must not restart the game after death
	while game.frame_count() == 0 or game.last_frame()[10].rindex("O") < 17:
		game._pump(0.01)
	os.write(game.master, b"r")
	game.wait_for("GAME OVER!")
	game.wait_for("Quit")
	game.drain()
	frames = game.frame_count()
	size = len(game.out)
	game.send(b"".join(UNREGISTERED_KEYS) + b"wasd" + b"\x1b[A\x1b[B\x1b[C\x1b[D")
	game.wait_input_consumed()
	game.drain()
	assert game.proc.poll() is None, "process exited"
	assert game.frame_count() == frames, "a frame was drawn on the panel"
	assert len(game.out) == size, "output after the panel: %r" % game.out[size:]
	game.send("r")
	game.wait_frames(frames + 1)
	quit_game(game)
