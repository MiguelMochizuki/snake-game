"""
test_gameover_ignores_keys.py
Author: Miguel Mochizuki Silva
Description: E2E: on the game-over panel only r and q act; r typed just before death does not restart
"""
import os
import sys
import tempfile

from harness import Game, UNREGISTERED_KEYS, quit_game, strip_ansi

# 100 ms frames; the head starts at x=10 and dies on the move past x=19
with tempfile.TemporaryDirectory() as tmp, Game(sys.argv[1], cwd=tmp, frame_ms=100) as game:
	game.wait_frames(2)
	# Stream r (one byte per 0.2 ms pump, ~500 per frame) once the head is near the wall, so some
	# land between the fatal tick's input poll and the flush; without the flush one would restart the game
	while "GAME OVER!" not in strip_ansi(game.out):
		if game.frame_count() and game.last_frame()[10].rindex("O") >= 17:
			os.write(game.master, b"r")
		game._pump(0.0002)
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
