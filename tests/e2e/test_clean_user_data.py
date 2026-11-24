"""
test_clean_user_data.py
Author: Miguel Mochizuki Silva
Description: Runs tools/clean_user_data.sh on a temporary data home and checks it removes only the two score folders
"""
import os
import subprocess
import sys
import tempfile

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
SCRIPT = os.path.join(ROOT, "tools", "clean_user_data.sh")


def test_removes_both_score_folders_and_nothing_else():
	"""
	Checks the graphical and terminal score folders go, while a neighbor folder, a file of the same family and the
	rest of the data home stay
	"""
	home = tempfile.mkdtemp()
	for name in ("snake-game", "snake-game-terminal", "snake-game-other"):
		os.makedirs(os.path.join(home, name))
		with open(os.path.join(home, name, "scores_v2.txt"), "w") as f:
			f.write("N 5\n")
	with open(os.path.join(home, "snake-game.txt"), "w") as f:
		f.write("keep")
	subprocess.run(["bash", SCRIPT, home], check=True, stdout=subprocess.DEVNULL)
	assert not os.path.exists(os.path.join(home, "snake-game"))
	assert not os.path.exists(os.path.join(home, "snake-game-terminal"))
	assert os.path.exists(os.path.join(home, "snake-game-other", "scores_v2.txt"))
	assert os.path.exists(os.path.join(home, "snake-game.txt"))
	# Nothing to remove is not an error
	subprocess.run(["bash", SCRIPT, home], check=True, stdout=subprocess.DEVNULL)


if __name__ == "__main__":
	test_removes_both_score_folders_and_nothing_else()
	print("ok")
	sys.exit(0)
