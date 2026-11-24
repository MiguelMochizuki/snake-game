"""
test_data_dir.py
Author: Miguel Mochizuki Silva
Description: E2E: scores go to SNAKE_DATA_DIR, else to ./data when it already holds scores, else to the user's data folder; a folder that cannot be written is reported on the menu and the panel
"""
import os
import sys
import tempfile

from harness import Game, end_game

MENU = {"SNAKE_SKIP_MENU": ""}


def run(cwd, env):
	"""
	Plays one game to its end

	Args:
		cwd (str): Working directory
		env (dict): Extra environment variables

	Returns:
		str: Stripped game-over panel text
	"""
	with Game(sys.argv[1], cwd=cwd, env=env) as game:
		return end_game(game)


with tempfile.TemporaryDirectory() as tmp:
	home = os.path.join(tmp, "home")
	work = os.path.join(tmp, "work")
	os.mkdir(home)
	os.mkdir(work)
	base = {"HOME": home, "XDG_DATA_HOME": "", "SNAKE_DATA_DIR": ""}

	# Nothing in ./data: the scores go to the user's data folder, whatever the current directory is
	run(work, base)
	assert os.path.exists(os.path.join(home, ".local", "share", "snake-game-terminal", "scores_v2.txt"))
	assert not os.path.exists(os.path.join(work, "data"))

	# The override beats everything
	chosen = os.path.join(tmp, "chosen")
	run(work, dict(base, SNAKE_DATA_DIR=chosen))
	assert os.path.exists(os.path.join(chosen, "scores_v2.txt"))

	# Scores already in ./data stay where they are
	os.mkdir(os.path.join(work, "data"))
	open(os.path.join(work, "data", "scores_v2.txt"), "w").close()
	run(work, base)
	with open(os.path.join(work, "data", "scores_v2.txt")) as f:
		assert f.read() == "N 0\n"

	# A folder that cannot be written (a plain file is in the way) is reported on the menu and on the panel
	blocked = os.path.join(tmp, "blocked")
	open(blocked, "w").close()
	with Game(sys.argv[1], cwd=work, env={**base, **MENU, "SNAKE_DATA_DIR": blocked}) as game:
		game.wait_for(r"Scores cannot be saved: " + blocked)
		game.send_now("\r")
		panel = end_game(game)
		assert "Scores not saved" in panel, panel
