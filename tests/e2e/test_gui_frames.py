"""
test_gui_frames.py
Author: Miguel Mochizuki Silva
Description: E2E: snake-gui --demo --frames draws off-screen without a display and writes the same frames for the same seed
"""
import glob
import os
import subprocess
import sys
import tempfile


def run(binary, out_dir):
	env = {k: v for k, v in os.environ.items() if k not in ("DISPLAY", "WAYLAND_DISPLAY", "SDL_VIDEODRIVER")}
	env.update(SNAKE_SEED="1", SNAKE_AUDIO="off")
	with tempfile.TemporaryDirectory() as cwd:
		result = subprocess.run([binary, "--demo", "--frames", out_dir, "--stride", "5", "--max-steps", "200"], cwd=cwd, env=env, capture_output=True, timeout=120)
	assert result.returncode == 0, result.stderr.decode()
	return sorted(glob.glob(os.path.join(out_dir, "frame-*.ppm")))


with tempfile.TemporaryDirectory() as a, tempfile.TemporaryDirectory() as b:
	first = run(sys.argv[1], a)
	second = run(sys.argv[1], b)
	assert len(first) >= 10, "only %d frames" % len(first)
	assert [os.path.basename(f) for f in first] == [os.path.basename(f) for f in second]
	assert os.path.basename(first[0]) == "frame-00000.ppm"
	for f in (first[0], first[-1]):
		with open(f, "rb") as fh:
			assert fh.read(2) == b"P6", f
	for x, y in ((first[0], second[0]), (first[len(first) // 2], second[len(second) // 2]), (first[-1], second[-1])):
		with open(x, "rb") as fx, open(y, "rb") as fy:
			assert fx.read() == fy.read(), "frames differ between runs: " + os.path.basename(x)
