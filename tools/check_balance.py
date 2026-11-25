"""
check_balance.py
Author: Miguel Mochizuki Silva
Description: Checks the mix of every piece of the soundtrack, as the game's synth plays it (make music), against the
balance rules:
	1. no clipping: the sample peak of every piece is at most -3 dBFS;
	2. a readable tune: the A-weighted level of the melody played alone is 3 to 8 dB above the accompaniment's;
	3. registers (a warning only): A-weighted bands against the middle (250-2000 Hz): the bass (60-250 Hz) at -12 dB
	   +-3 and the treble (2-8 kHz) at -5 dB +-2.
It also shows the integrated loudness (target -20.2 LUFS, the "gain" of each piece in tools/gen_score.py), measured
with ffmpeg. Prints a table and exits with 1 when a rule 1 or 2 fails.
Usage: python3 tools/check_balance.py [--tool build-music/bin/snake_render_music] [--music music] [--stems DIR]
"""
import argparse
import glob
import os
import re
import subprocess
import sys
import wave

import numpy as np
from scipy.signal import welch

PEAK_MAX = -3.0
MELODY_MIN, MELODY_MAX = 3.0, 8.0
BASS, BASS_TOLERANCE = -12.0, 3.0
TREBLE, TREBLE_TOLERANCE = -5.0, 2.0
LUFS, LUFS_TOLERANCE = -20.2, 0.3


def read(path):
	"""Reads a mono 16-bit WAV file.

	Args:
		path: The file.

	Returns:
		(samples in [-1, 1], sample rate).
	"""
	with wave.open(path) as w:
		data = np.frombuffer(w.readframes(w.getnframes()), dtype="<i2").astype(float) / 32768.0
		return data, w.getframerate()


def a_weighting(f):
	"""Power gain of the A-weighting curve (IEC 61672), 1 at 1 kHz.

	Args:
		f: Frequencies in Hz (numpy array).

	Returns:
		The power gain at each frequency.
	"""
	f2 = np.maximum(f, 1.0) ** 2
	ra = 12194.0 ** 2 * f2 ** 2 / ((f2 + 20.6 ** 2) * np.sqrt((f2 + 107.7 ** 2) * (f2 + 737.9 ** 2)) * (f2 + 12194.0 ** 2))
	return (ra / 0.7943) ** 2


def spectrum(samples, rate):
	"""A-weighted power spectral density of the sound.

	Args:
		samples: The samples.
		rate: Sample rate in Hz.

	Returns:
		(frequencies, power density).
	"""
	f, p = welch(samples, rate, nperseg=8192)
	return f, p * a_weighting(f)


def level(f, p, lo=0.0, hi=1e9):
	"""A-weighted level in dB of a band of a spectrum.

	Args:
		f: Frequencies of the spectrum.
		p: Its power density.
		lo: Lowest frequency of the band.
		hi: Highest frequency of the band.

	Returns:
		The level in dB.
	"""
	band = (f >= lo) & (f < hi)
	return 10.0 * np.log10(np.trapezoid(p[band], f[band]) + 1e-20)


def loudness(path):
	"""Integrated loudness (EBU R 128) of a file, measured by ffmpeg.

	Args:
		path: The file.

	Returns:
		The loudness in LUFS, or None without ffmpeg.
	"""
	try:
		out = subprocess.run(["ffmpeg", "-nostats", "-i", path, "-af", "ebur128", "-f", "null", "-"],
			capture_output=True, text=True).stderr
	except FileNotFoundError:
		return None
	found = re.findall(r"I:\s+(-?[\d.]+) LUFS", out)
	return float(found[-1]) if found else None


def render(tool, out, stem=None):
	"""Writes every piece as a WAV file with the game's synth.

	Args:
		tool: The snake_render_music executable.
		out: Folder of the files (emptied of WAV files first).
		stem: "melody" or "accompaniment" to play only those notes, None for both.
	"""
	os.makedirs(out, exist_ok=True)
	for f in glob.glob(os.path.join(out, "*.wav")):
		os.remove(f)
	subprocess.run([tool, out] + ([stem] if stem else []), check=True, stdout=subprocess.DEVNULL)


def main():
	"""Renders the stems, measures every piece and prints the table."""
	ap = argparse.ArgumentParser()
	ap.add_argument("--tool", default="build-music/bin/snake_render_music")
	ap.add_argument("--music", default="music")
	ap.add_argument("--stems", default="build-music/stems")
	args = ap.parse_args()
	render(args.tool, os.path.join(args.stems, "melody"), "melody")
	render(args.tool, os.path.join(args.stems, "accompaniment"), "accompaniment")
	failed, warned = [], []
	print("%-46s %7s %7s %8s %7s %7s" % ("piece", "peak", "LUFS", "mel-acc", "bass", "treble"))
	print("%-46s %7s %7s %8s %7s %7s" % ("", "<=-3", "-20.2", "3..8", "-12+-3", "-5+-2"))
	for path in sorted(glob.glob(os.path.join(args.music, "*.wav"))):
		name = os.path.basename(path)
		mix, rate = read(path)
		peak = 20.0 * np.log10(np.abs(mix).max() + 1e-20)
		f, p = spectrum(mix, rate)
		mid = level(f, p, 250, 2000)
		bass, treble = level(f, p, 60, 250) - mid, level(f, p, 2000, 8000) - mid
		melody = level(*spectrum(*read(os.path.join(args.stems, "melody", name))))
		accompaniment = level(*spectrum(*read(os.path.join(args.stems, "accompaniment", name))))
		lufs = loudness(path)
		marks = []
		if peak > PEAK_MAX:
			failed.append("%s: peak %.1f dBFS" % (name, peak))
			marks.append("PEAK")
		if not MELODY_MIN <= melody - accompaniment <= MELODY_MAX:
			failed.append("%s: melody - accompaniment %+.1f dB" % (name, melody - accompaniment))
			marks.append("MELODY")
		if abs(bass - BASS) > BASS_TOLERANCE:
			warned.append("%s: bass %+.1f dB" % (name, bass))
			marks.append("bass")
		if abs(treble - TREBLE) > TREBLE_TOLERANCE:
			warned.append("%s: treble %+.1f dB" % (name, treble))
			marks.append("treble")
		if lufs is not None and abs(lufs - LUFS) > LUFS_TOLERANCE:
			warned.append("%s: %.1f LUFS" % (name, lufs))
			marks.append("lufs")
		print("%-46s %7.1f %7s %+8.1f %+7.1f %+7.1f  %s" % (name[:46], peak, "%.1f" % lufs if lufs is not None else "-",
			melody - accompaniment, bass, treble, " ".join(marks)))
	for w in warned:
		print("warning: " + w)
	for e in failed:
		print("FAILED: " + e)
	sys.exit(1 if failed else 0)


if __name__ == "__main__":
	main()
