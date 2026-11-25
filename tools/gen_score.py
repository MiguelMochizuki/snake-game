"""
gen_score.py
Author: Miguel Mochizuki Silva
Description: Writes src/audio/score_data.cpp (the game's soundtrack). Every piece is a list of notes (start and length in
milliseconds, MIDI pitch, melody or accompaniment, velocity) built from a MIDI file or a MusicXML file. An arrangement
layer then plays the piece the way it is performed: repeats written out, a tempo, a ritardando, a dynamics curve, a
balance between the accompaniment and the melody and a volume by register; no piece needs more voices than the synth has.
Usage: python3 tools/gen_score.py [--cache DIR] [--out src/audio/score_data.cpp]
The files with a stable URL (Mutopia, Wikimedia Commons) are downloaded into the cache folder (default: a temporary
folder) when they are not there yet; the others are in tools/sources/.
"""
import argparse
import hashlib
import math
import os
import re
import struct
import tempfile
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
import zipfile

USER_AGENT = "snake-game-score-generator/2.0"
MUTOPIA = "https://www.mutopiaproject.org/ftp/"
STEPS = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}
VOICES = 16  # the synth's MAX_VOICES
DYNAMICS = {"pppp": 16, "ppp": 24, "pp": 36, "p": 52, "mp": 66, "mf": 82, "f": 98, "ff": 112, "fff": 124,
			"sf": 112, "sfz": 112, "fp": 90, "rf": 100, "rfz": 100}


def commons(name):
	"""URL of a file of Wikimedia Commons."""
	name = name.replace(" ", "_")
	digest = hashlib.md5(name.encode()).hexdigest()
	return "https://upload.wikimedia.org/wikipedia/commons/%s/%s/%s" % (digest[0], digest[:2], urllib.parse.quote(name))


# Notes are (start, length, pitch, velocity, is_melody) with times in beats (a beat is a quarter note); MusicXML notes
# carry a sixth item, the part they come from.

def read_vlq(data, i):
	"""Reads a MIDI variable-length quantity; returns (value, next index)."""
	value = 0
	while True:
		b = data[i]
		i += 1
		value = (value << 7) | (b & 0x7F)
		if not b & 0x80:
			return value, i


def parse_midi(path):
	"""Returns (ticks per quarter note, tempo changes, tracks).

	The tempo changes are (tick, microseconds per quarter note); each track is a list of (tick, status, args).
	"""
	data = open(path, "rb").read()
	_, _, _, count, division = struct.unpack(">4sIHHH", data[:14])
	i, tracks, tempos = 14, [], []
	for _ in range(count):
		_, length = struct.unpack(">4sI", data[i:i + 8])
		i += 8
		end, tick, running, events = i + length, 0, 0, []
		while i < end:
			delta, i = read_vlq(data, i)
			tick += delta
			b = data[i]
			if b == 0xFF:
				size, j = read_vlq(data, i + 2)
				if data[i + 1] == 0x51 and size == 3:
					tempos.append((tick, int.from_bytes(data[j:j + 3], "big")))
				i = j + size
				continue
			if b in (0xF0, 0xF7):
				size, j = read_vlq(data, i + 1)
				i = j + size
				continue
			if b & 0x80:
				running = b
				i += 1
			kind = running & 0xF0
			if kind in (0xC0, 0xD0):
				args, i = (data[i],), i + 1
			else:
				args, i = (data[i], data[i + 1]), i + 2
			events.append((tick, kind, args))
		tracks.append(events)
	return division, sorted(tempos), tracks


def midi_notes(track):
	"""Returns the notes of a MIDI track as sorted (start tick, end tick, pitch, velocity).

	A note-on of a pitch that is already sounding ends the note before it (otherwise that note would be lost).
	"""
	on, notes = {}, []
	for tick, kind, args in track:
		if kind == 0x90 and args[1] > 0:
			if args[0] in on and on[args[0]][0] < tick:
				start, velocity = on[args[0]]
				notes.append((start, tick, args[0], velocity))
			on[args[0]] = (tick, args[1])
		elif kind == 0x80 or (kind == 0x90 and args[1] == 0):
			if args[0] in on:
				start, velocity = on.pop(args[0])
				notes.append((start, tick, args[0], velocity))
	return sorted(notes)


def skyline(notes):
	"""Splits notes into (melody, rest): a note is melody when no note sounding at its start is higher."""
	melody, rest = [], []
	for n in notes:
		higher = any(o[0] <= n[0] < o[1] and o[2] > n[2] for o in notes)
		(rest if higher else melody).append(n)
	return melody, rest


def register_gain(pitch):
	"""Volume factor of a note by its register: flat from C3 to C6, rising to +4 dB at F#2 and below (a low triangle
	wave sounds weak) and falling to -2 dB at F#6 and above (a high square wave sounds shrill)."""
	db = 4.0 * min(1.0, max(0.0, (48 - pitch) / 6.0)) - 2.0 * min(1.0, max(0.0, (pitch - 84) / 6.0))
	return 10 ** (db / 20)


def no_restrikes(notes):
	"""Ends a note where the same pitch of the same line (melody or accompaniment) starts again, and drops a note that
	starts together with another of the same pitch: two voices on one pitch beat against each other in the synth."""
	out, ringing = [], {}
	for n in sorted(notes, key=lambda n: (n[0], -n[1])):
		key = (n[2], n[4])
		if key in ringing:
			k = ringing[key]
			if out[k][0] == n[0]:
				continue
			if out[k][0] + out[k][1] > n[0]:
				out[k] = (out[k][0], n[0] - out[k][0]) + out[k][2:]
		ringing[key] = len(out)
		out.append(n)
	return out


def limit_voices(notes, limit):
	"""Drops accompaniment notes that would make more than `limit` notes sound at once; the melody is always kept.

	The game's synth has 16 voices and takes the oldest one when they run out, which cuts a note off that still rings.
	"""
	kept, active = [], []
	for n in sorted(notes, key=lambda n: (n[0], not n[4])):
		active = [e for e in active if e > n[0] + 1e-6]
		if n[4] or len(active) < limit:
			kept.append(n)
			active.append(n[0] + n[1])
	return kept


def load_midi(spec, path):
	"""Reads a MIDI piece: returns (notes, beats per bar, tempo in BPM from the file)."""
	division, tempos, tracks = parse_midi(path)
	for k, semitones in spec.get("transpose", {}).items():
		tracks[k] = [(t, kind, (args[0] + semitones, args[1]) if kind in (0x80, 0x90) else args) for t, kind, args in tracks[k]]
	sig = re.search(b"\xff\x58\x04(....)", open(path, "rb").read())
	beats_per_bar = sig.group(1)[0] * 4.0 / 2 ** sig.group(1)[1] if sig else 4.0
	melody, rest = [], []
	if "melody_ranges" in spec:
		# The tune passes from one instrument to another: (first tick, last tick, tracks) says who carries it, and
		# those tracks do not also play the accompaniment while they carry it
		for lo, hi, ks in spec["melody_ranges"]:
			m, r = skyline([n for k in ks for n in midi_notes(tracks[k]) if lo <= n[0] < hi])
			melody += m
			rest += r
		for k in spec["accompaniment"]:
			rest += [n for n in midi_notes(tracks[k])
					 if not any(lo <= n[0] < hi and k in ks for lo, hi, ks in spec["melody_ranges"])]
	else:
		for k in spec["melody"]:
			m, r = skyline(midi_notes(tracks[k]))
			melody += m
			rest += r
		for k in spec.get("accompaniment", [k for k in range(1, len(tracks)) if k not in spec["melody"]]):
			rest += midi_notes(tracks[k])
	if spec.get("performance"):
		# A recorded performance: its tempo changes are the interpretation, so keep them. Time is counted in seconds
		# (a beat of 60 BPM) and "speed" scales it
		marks, seconds, last, tempo = [], 0.0, 0, 500000
		for tick, t in [(0, 500000)] + tempos:
			seconds += (tick - last) * tempo / division / 1e6
			marks.append((tick, seconds))
			last, tempo = tick, t

		def at(tick):
			k = max(i for i, m in enumerate(marks) if m[0] <= tick) if tick < marks[-1][0] else len(marks) - 1
			return marks[k][1] + (tick - marks[k][0]) * tempo_of[k] / division / 1e6

		tempo_of = [500000] + [t for _, t in tempos]
		cut = spec.get("cut", 1e9)
		timed = [(at(s), at(e) - at(s), p, v, m) for group, m in ((melody, True), (rest, False)) for s, e, p, v in group]
		if "window" in spec:
			lo, hi = at(spec["window"][0]), at(spec["window"][1])
			timed = [(s - lo, min(l, hi - s), p, v, m) for s, l, p, v, m in timed if lo <= s < hi]
		if "voices" in spec:
			timed = limit_voices(timed, spec["voices"])
		return [n for n in timed if n[0] < cut], 4.0, 60.0
	notes = [(s / division, (e - s) / division, p, v, True) for s, e, p, v in melody]
	notes += [(s / division, (e - s) / division, p, v, False) for s, e, p, v in rest]
	if spec.get("quantize"):
		# A live recording: snap starts and ends to a grid (in beats) and drop the ghost notes it left
		q = spec["quantize"]
		notes = [(round(s / q) * q, max(q, round(l / q) * q), p, v, m) for s, l, p, v, m in notes if l >= 0.1]
	return notes, beats_per_bar, 60000000.0 / tempos[0][1] if tempos else 120.0


def unfold_measures(measures):
	"""Returns the order in which the measures are played, following repeat signs and numbered endings."""
	order, i, start, passes = [], 0, 0, 1
	while i < len(measures):
		m = measures[i]
		if m["forward"] and i != start:
			start, passes = i, 1
		if m["ending_start"] and passes not in m["ending_start"]:
			while not measures[i]["ending_stop"]:
				i += 1
			i += 1
			continue
		order.append(i)
		if m["backward"] and passes == 1:
			passes = 2
			i = start
			continue
		if m["backward"] or m["ending_start"]:
			passes, start = 1, i + 1
		i += 1
	return order


def read_measure(el, state):
	"""Reads one MusicXML measure into a dict of notes (in beats from its start), direction marks and repeat flags."""
	info = {"forward": False, "backward": False, "ending_start": set(), "ending_stop": False, "notes": [], "dyn": [],
			"length": 0.0}
	pos, last, tied = 0, 0, {}
	for e in el:
		if e.tag == "attributes":
			if e.find("divisions") is not None:
				state["divisions"] = int(e.find("divisions").text)
			t = e.find("time")
			if t is not None:
				state["bar"] = int(t.find("beats").text) * 4.0 / int(t.find("beat-type").text)
		elif e.tag == "barline":
			r = e.find("repeat")
			if r is not None:
				info["forward" if r.get("direction") == "forward" else "backward"] = True
			end = e.find("ending")
			if end is not None:
				if end.get("type") == "start":
					info["ending_start"] = {int(x) for x in re.findall(r"\d+", end.get("number"))}
				else:
					info["ending_stop"] = True
		elif e.tag == "direction":
			for o in e.iter("octave-shift"):
				# 8va: the notes are written lower than they sound ("down"); the bracket belongs to the staff it is on
				size = 24 if o.get("size") == "15" else 12
				staff = e.findtext("staff") or "1"
				state.setdefault("shift", {})[staff] = {"down": size, "up": -size}.get(o.get("type"), 0)
			off = e.find("offset")
			at = (pos + (int(off.text) if off is not None else 0)) / state["divisions"]
			for d in e.iter("dynamics"):
				for c in d:
					if c.tag in DYNAMICS:
						info["dyn"].append((at, "set", DYNAMICS[c.tag]))
			for w in e.iter("wedge"):
				info["dyn"].append((at, "wedge", w.get("type")))
			for s in e.iter("sound"):
				if s.get("tempo") and state["bpm"] is None:
					state["bpm"] = float(s.get("tempo"))
		elif e.tag == "backup":
			pos -= int(e.find("duration").text)
		elif e.tag == "forward":
			pos += int(e.find("duration").text)
		elif e.tag == "note" and e.find("grace") is None:
			duration = int(e.find("duration").text)
			chord = e.find("chord") is not None
			start = last if chord else pos
			if not chord:
				last = pos
				pos += duration
			info["length"] = max(info["length"], pos / state["divisions"])
			p = e.find("pitch")
			if p is None:
				continue
			staff = e.findtext("staff") or "1"
			midi = 12 * (int(p.find("octave").text) + 1) + STEPS[p.find("step").text] + state.get("shift", {}).get(staff, 0)
			if p.find("alter") is not None:
				midi += int(float(p.find("alter").text))
			ties = {t.get("type") for t in e.findall("tie")}
			if "stop" in ties and midi in tied:
				tied[midi]["len"] += duration / state["divisions"]
				if "start" not in ties:
					del tied[midi]
				continue
			n = {"start": start / state["divisions"], "len": duration / state["divisions"], "pitch": midi, "staff": staff,
				 "voice": e.findtext("voice") or "1"}
			info["notes"].append(n)
			if "start" in ties:
				tied[midi] = n
	return info


def load_musicxml(spec, path):
	"""Reads a MusicXML piece (compressed or not), unfolding its repeats.

	Returns (notes, beats per bar, tempo in BPM, dynamics) where dynamics maps a part to (beat, velocity) points.
	"""
	if path.endswith(".mxl"):
		z = zipfile.ZipFile(path)
		root = ET.fromstring(z.read([n for n in z.namelist() if n.endswith(".xml") and not n.startswith("META")][0]))
	else:
		root = ET.parse(path).getroot()
	notes, dynamics, order = [], {}, None
	state = {"divisions": 1, "bar": 4.0, "bpm": None}
	for index, part in enumerate(root.findall("part")):
		elements = part.findall("measure")
		measures = [read_measure(m, state) for m in elements]
		if "order" in spec:
			# The route through the score written out (da capo, dal segno, coda and Fine are not in the file): ranges
			# of measure numbers, the last one optionally stopping after some beats of its last measure
			number = {m.get("number"): i for i, m in enumerate(elements)}
			order = [(i, r[2] if len(r) > 2 and k == r[1] else None) for r in spec["order"]
					 for k in range(r[0], r[1] + 1) for i in [number[str(k)]]]
		order = order or [(i, None) for i in unfold_measures(measures)]
		beat, points, ramp = 0.0, [], None
		for mi, until in order:
			m = measures[mi]
			for n in m["notes"]:
				if until is not None and n["start"] >= until - 1e-6:
					continue
				tune = (index == spec["melody"][0] and n["staff"] == str(spec.get("melody_staff", n["staff"]))
						and n["voice"] == spec.get("melody_voice", n["voice"]))
				low = int(elements[mi].get("number")) in spec.get("melody_low", ())
				notes.append((beat + n["start"], n["len"], n["pitch"], 0, tune, index, low))
			for at, kind, value in sorted(m["dyn"]):
				if kind == "set":
					points.append((beat + at, value))
				elif value in ("crescendo", "diminuendo"):
					ramp = (beat + at, points[-1][1] if points else 64, 22 if value == "crescendo" else -22)
				elif ramp:
					points += [(ramp[0], ramp[1]), (beat + at, ramp[1] + ramp[2])]
					ramp = None
			beat += until if until is not None else (m["length"] or state["bar"])
		dynamics[index] = sorted(points)
	if spec.get("skyline"):
		# a piano line: of the notes of the tune's staff that sound together, only the highest is the melody (the
		# lowest in the bars of "melody_low", where the tune has a harmony note above it)
		top = {}
		for n in notes:
			if n[4]:
				key = round(n[0], 3)
				top[key] = (min if n[6] else max)(top.get(key, n[2]), n[2])
		notes = [n[:4] + (n[4] and n[2] == top[round(n[0], 3)],) + n[5:] for n in notes]
	notes = [n[:6] for n in notes]
	return notes, spec.get("bar", state["bar"]), state["bpm"] or 72.0, dynamics


# The repertoire, in the order of score_data.cpp.
#   url/file: where the notes come from: a stable URL (downloaded into the cache) or a file of tools/sources/, for
#             the ones that have no stable URL (see tools/README.md)
#   source:   "midi" or "musicxml"
#   melody:   MIDI tracks (or the MusicXML part) whose highest note at any time is the melody
#   form:     bars (1-based, inclusive; fractions start or stop inside a bar) in the order they are played; the repeats
#             and their endings are written out here
#   bpm:      tempo that replaces the one of the file; rit: the last bars slow down to a fraction of the tempo
#   dyn:      (bar of the form, level 0-100) points of the volume curve, joined by straight lines
#   accompaniment_gain: (first bar, last bar, factor) changes of the accompaniment's volume
#   melody_db: the melody against the accompaniment, in dB (tools/check_balance.py measures it)
#   gain:     scales every velocity so that all pieces sound equally loud (-20.2 LUFS integrated, measured by
#             make loudness on the output of make music); gain and melody_db are set by measuring (make balance)
#   transpose: {MIDI track: semitones}
#   performance/window/voices: a recorded performance, whose tempo changes are kept; the window (ticks) taken from
#             it; at most so many notes at once
#   melody_ranges: (first tick, last tick, tracks): the tracks that carry the tune in that stretch
#   MusicXML: melody_staff/melody_voice: where the tune is; skyline: the highest note of it is the tune (the lowest
#             in the bars of melody_low); bar: beats per bar when the file has no time signature; order: the route
#             through the measures, (first, last[, beats of the last one]) in printed bar numbers
#   cap/last: Ave Maria's prelude is an arpeggio of one note per sixteenth, with a chord that rings out at the end
#   ring:     the notes of the last chord last at least so many beats
#   rare:     an easter egg, chosen with 1% of the probability each

PIECES = [
	{"title": "Ave Maria", "gain": 0.923, "melody_db": -2.0, "composer": "C. Gounod, over J. S. Bach's Prelude in C major", "mutopia": 2167,
	 "from": "Heugel, 1856, typeset by Yann Leprince, public domain",
	 "source": "midi", "url": MUTOPIA + "GounodC/AveMaria-PianoViolin/AveMaria-PianoViolin.mid",
	 "melody": [1], "accompaniment": [2, 3], "cap": 0.25, "last": 4.0, "bpm": 66, "rit": {"bars": 3, "factor": 0.45},
	 "dyn": [(1, 62), (16, 70), (36, 78), (60, 90), (68, 100), (73, 100), (74, 85)],
	 # the bass of the last bars is brought up to the melody (1.7 before the velocities had room; 1.4 keeps the peak
	 # of the climax about where it was)
	 "accompaniment_gain": [(66, 74, 1.4)]},
	{"title": "Air on the G String", "gain": 1.188, "melody_db": -8.0, "composer": "J. S. Bach", "mutopia": 1534,
	 "from": "Orchestral Suite No. 3, BWV 1068, arranged for flute and guitar after the Bach-Gesellschaft "
			"edition, typeset by Mike Blackstock, public domain",
	 "source": "midi", "url": MUTOPIA + "BachJS/BWV1068/bach_air_bmv_1068/bach_air_bmv_1068.mid", "melody": [1],
	 # an arrangement for flute (track 1) and guitar (track 2); the guitar is written an octave above the sound, as
	 # guitar music is, but the MIDI file plays it as written, so it is put back down an octave
	 "transpose": {2: -12},
	 # The theme (bars 1-5) is played twice: bar 6 is the first ending, which runs back to the start, and bars 7-8 are the
	 # second, which go on into the development (bars 9-19, also repeated)
	 "form": [(1, 5), (6, 6), (1, 5), (7, 8), (9, 19), (9, 19)], "bpm": 44, "rit": {"bars": 2, "factor": 0.55},
	 # each section swells towards its middle and settles at its end; the repeats are a little softer, like an echo
	 "dyn": [(1, 66), (4, 76), (6, 68), (7, 64), (10, 74), (13, 66), (14, 68), (19, 82), (24, 70), (25, 64), (30, 78),
			 (34, 66), (35, 52)]},
	{"title": "Eine kleine Nachtmusik, 1st movement", "gain": 0.676, "melody_db": -2.4, "composer": "W. A. Mozart", "mutopia": 900,
	 "from": "Serenade K. 525, from IMSLP, typeset anonymously, public domain",
	 "source": "midi", "url": MUTOPIA + "MozartWA/KV525/eine-kleine-nachtmusik-mvt1/eine-kleine-nachtmusik-mvt1.mid",
	 # Allegro: quarter note 152, between the brisk and the classic recordings (168 was faster than any)
	 "melody": [1], "bpm": 152, "rit": {"bars": 2, "factor": 0.8}},
	{"title": "Rondo alla Turca", "gain": 0.750, "melody_db": -4.0, "composer": "W. A. Mozart", "mutopia": 108,
	 "from": "Piano Sonata K. 331, 3rd movement, from IMSLP, typeset by Rune Zedeler and Chris Sawer, public domain",
	 "source": "midi", "url": MUTOPIA + "MozartWA/KV331/KV331_3_RondoAllaTurca/KV331_3_RondoAllaTurca.mid",
	 "melody": [1], "bpm": 132, "rit": {"bars": 2, "factor": 0.7},
	 # The MIDI file plays every repeat once: they are written out here. Bars of 2 beats counted from the pickup, so a
	 # section of the score (pickup to the quarter note that ends it) is a whole number of them: A (1-8), B A' (9-24),
	 # the A major march (25-32), F sharp minor (33-40), E (41-56), the march, A, B A', each twice, then the coda's
	 # march with its first ending (89-96), again with the second (89-95.5, then 97 on) and the end
	 "form": [(1, 8), (1, 8), (9, 24), (9, 24), (25, 32), (25, 32), (33, 40), (33, 40), (41, 56), (41, 56), (57, 64),
			  (57, 64), (65, 72), (65, 72), (73, 88), (73, 88), (89, 96), (89, 95.5), (97, 128)]},
	{"title": "Ode to Joy, finale of the 9th symphony", "gain": 0.944, "melody_db": -3.1, "composer": "L. van Beethoven",
	 "from": "4th movement, bars 92-207: the theme in the basses, the violas, the violins and the whole orchestra; "
			 "Wikimedia Commons, Sym 9 Fourth Movement.mid, sequenced by D. L. Viens in 1998 and revised by wangyichi, "
			 "public domain",
	 "source": "midi", "url": commons("Sym 9 Fourth Movement.mid"), "performance": True,
	 # ticks of bars 92 and 208; the tune is in the cellos, then the violas and bassoons, then the violins
	 "window": (32760, 88440), "voices": 12,
	 "melody_ranges": [(32760, 32760 + 24 * 480, [30]), (32760 + 24 * 480, 32760 + 48 * 480, [29, 9]),
					   (32760 + 48 * 480, 88440, [27])],
	 "accompaniment": [28, 29, 30, 31, 11, 17], "melody": [30], "bpm": 60, "rit": {"bars": 1, "factor": 0.8}},
	{"title": "Standchen (Leise flehen meine Lieder)", "gain": 0.794, "melody_db": -3.9, "composer": "F. Schubert",
	 "from": "Schwanengesang D. 957 No. 4, arranged for solo and chords; MIDI file of MIDIfind.com (midifind.com), "
			 "author and license not confirmed",
	 "source": "midi", "file": "sources/standchen_midifind.mid", "melody": [0], "accompaniment": [1],
	 "bpm": 90, "rit": {"bars": 2, "factor": 0.6}},
	{"title": "La Marseillaise", "gain": 0.766, "melody_db": -3.7, "composer": "C. J. Rouget de Lisle",
	 "from": "a sequence of Online Sequencer (onlinesequencer.net) rewritten by hand bar by bar "
			 "(tools/sources/marseillaise_fix.py), license not confirmed",
	 "source": "midi", "file": "sources/marseillaise_corrected.mid", "melody": [0], "accompaniment": [1],
	 # the anthem twice: the last beat of the first verse is already the pickup of the second
	 "form": [(1.5, 28.75), (1.75, 29)], "bpm": 116, "rit": {"bars": 1, "factor": 0.6}, "rare": True},
	{"title": "Korobeiniki (the Tetris theme)", "gain": 0.616, "melody_db": -1.2, "composer": "traditional Russian folk song",
	 "from": "a chiptune MIDI file of nonstop2k.com, which seems to transcribe the Game Boy arrangement of Hirokazu "
			 "Tanaka (Nintendo, 1989), license not confirmed",
	 "source": "midi", "file": "sources/tetris_nonstop2k.mid",
	 # Square 2 has the tune, Square 1 the counter-melody, Wave the bass; from beat 64 the bridge's tune (E C D B C A
	 # G# B) is the upper line of tracks 4 and 5, under the A-E eighths of the Wave; track 3 is the noise channel, left
	 # out (and so are some inaudible ghost notes of the Wave after the end)
	 "melody_ranges": [(0, 64 * 128, [1]), (64 * 128, 10 ** 9, [4, 5])], "accompaniment": [0, 2, 4, 5],
	 # the file is one lap of the game's loop (A B A B, then the bridge, which ends on the dominant): the tune comes back
	 # once more to end on the tonic
	 "form": [(1, 24), (1, 8)], "bpm": 150, "rit": {"bars": 2, "factor": 0.6}, "rare": True},
	{"title": "Apanhei-te, cavaquinho", "gain": 0.668, "melody_db": -0.3, "composer": "E. Nazareth",
	 "from": "polka of 1914; Musica Brasilis edition (IMSLP 614247), CC BY-SA 4.0",
	 "source": "musicxml", "file": "sources/nazareth.mxl", "melody": [0], "melody_staff": 1, "melody_voice": "1",
	 "skyline": True, "bar": 2.0, "bpm": 112, "rit": {"bars": 2, "factor": 0.7},
	 # pickup, A A (endings 16 and 17), B B (endings 33 and 34), A, then bar 51 (bar 50 is "only to end") into the Trio,
	 # Trio twice (endings 67 and 68), D.S. al Fine: A, B and A once each (the last endings) to the Fine of bar 50
	 "order": [(0, 16), (1, 15), (17, 33), (18, 32), (34, 49), (51, 67), (52, 66), (68, 68), (1, 15), (17, 32),
			   (34, 50)]},
	{"title": "Tico-tico no fuba", "gain": 1.083, "melody_db": -4.6, "composer": "Zequinha de Abreu",
	 "from": "choro; Musica Brasilis edition (IMSLP 1040456), CC BY-SA 4.0",
	 "source": "musicxml", "file": "sources/tico.mxl", "melody": [0], "melody_staff": 1, "melody_voice": "1",
	 # in these bars the tune (F E A, F E G#) is the lower note of the right hand's chords
	 "skyline": True, "melody_low": [5, 6, 13, 14], "bpm": 120, "rit": {"bars": 2, "factor": 0.7}, "ring": 1.0,
	 # introduction, A A (endings 20 and 21), B B (endings 37 and 38; the 8va of its repeat is ad libitum, left out),
	 # D.S. al Coda: A to the coda sign, coda (bar 39), C C (endings 55 and 56), D.S. al Fine: A to the Fine, which is
	 # the chord on the second beat of bar 20
	 "order": [(1, 20), (5, 19), (21, 37), (22, 36), (38, 38), (5, 19), (39, 55), (40, 54), (56, 56), (5, 19),
			   (20, 20, 1.25)]},
]


LICENSES = """The notes of Apanhei-te, cavaquinho and Tico-tico no fuba are derived from the Musica Brasilis editions
(musicabrasilis.org.br; IMSLP 614247 and 1040456, https://imslp.org) and are licensed under CC BY-SA 4.0
(https://creativecommons.org/licenses/by-sa/4.0/); the rest of the project stays under the MIT license. The licenses
of the files the Standchen, La Marseillaise and Korobeiniki come from are not confirmed (see README.md)."""


def fetch(piece, cache):
	"""Returns the path of the piece's file: one of tools/sources/, or the one in the cache, downloaded first when needed."""
	if "file" in piece:
		return os.path.join(os.path.dirname(os.path.abspath(__file__)), piece["file"])
	path = os.path.join(cache, os.path.basename(urllib.parse.unquote(piece["url"])))
	if not os.path.exists(path):
		request = urllib.request.Request(piece["url"], headers={"User-Agent": USER_AGENT})
		with urllib.request.urlopen(request) as response, open(path, "wb") as out:
			out.write(response.read())
	return path


def interpolate(points, x):
	"""Value at x of a polyline given as sorted (x, y) points; flat before the first and after the last."""
	if x <= points[0][0]:
		return points[0][1]
	for (x0, y0), (x1, y1) in zip(points, points[1:]):
		if x <= x1:
			return y0 + (y1 - y0) * (x - x0) / (x1 - x0) if x1 > x0 else y1
	return points[-1][1]


def arrange(piece, notes, bar, file_bpm, dynamics=None):
	"""Applies the arrangement: form, volumes, tempo and ritardando.

	Returns ([(ms start, ms length, pitch, melody, velocity)], end ms).
	"""
	if "form" in piece:
		out, offset = [], 0.0
		for first, last in piece["form"]:
			lo, hi = (first - 1) * bar, last * bar
			# a note cannot ring past the end of its section, since the next one starts somewhere else in the music
			out += [(n[0] - lo + offset, min(n[1], hi - n[0])) + n[2:] for n in notes if lo <= n[0] < hi]
			offset += hi - lo
		notes, total = out, offset
	else:
		total = math.ceil(max(n[0] + n[1] for n in notes) / bar - 1e-6) * bar
	if dynamics:
		notes = [n[:3] + (interpolate(dynamics[n[5]], n[0]) if dynamics.get(n[5]) else 80,) + n[4:] for n in notes]
	top = max(n[3] for n in notes)
	curve = [((b - 1) * bar, level) for b, level in piece.get("dyn", [])]
	shaped = []
	for n in notes:
		s, l, p, v, m = n[:5]
		v = 100.0 * v / top * piece.get("gain", 1.0) * register_gain(p)
		if m:
			v *= 10 ** (piece.get("melody_db", 0.0) / 20)
		if curve:
			v *= interpolate(curve, s) / 100.0
		for first, last, factor in piece.get("accompaniment_gain", []):
			if not m and (first - 1) * bar <= s < last * bar:
				v *= factor
		shaped.append((s, l, p, max(1, min(127, round(v))), m))
	if "cap" in piece:
		last_start = max(n[0] for n in shaped if not n[4])
		shaped = [(s, piece["last"] if (not m and s == last_start) else (min(l, piece["cap"]) if not m else l), p, v, m)
				  for s, l, p, v, m in shaped]
	if "ring" in piece:
		# the last chord rings out instead of stopping short (a Fine on a staccato chord)
		last_start = max(n[0] for n in shaped)
		shaped = [(s, max(l, piece["ring"]) if s == last_start else l, p, v, m) for s, l, p, v, m in shaped]
	bpm, rit = piece.get("bpm", file_bpm), piece.get("rit")
	rit_start = total - rit["bars"] * bar if rit else total

	def seconds(beat):
		"""Time of a beat: constant tempo until the ritardando, then a straight line down to bpm * factor."""
		if beat <= rit_start:
			return beat * 60.0 / bpm
		t, steps = rit_start * 60.0 / bpm, 48
		width = (beat - rit_start) / steps
		for i in range(steps):
			x = min(1.0, (i + 0.5) * width / (total - rit_start))
			t += width * 60.0 / (bpm * (1.0 - (1.0 - rit["factor"]) * x))
		return t

	timed = []
	for s, l, p, v, m in shaped:
		a, b = seconds(s), seconds(s + l)
		timed.append((round(a * 1000), max(30, round((b - a) * 1000)), p, v, m))
	final = [(s, l, p, 1 if m else 0, v) for s, l, p, v, m in limit_voices(no_restrikes(timed), VOICES)]
	final.sort()
	return final, max(a + b for a, b, _, _, _ in final)


def build(piece, cache):
	"""Returns the finished notes and end time of a piece."""
	path = fetch(piece, cache)
	if piece["source"] == "midi":
		notes, bar, bpm = load_midi(piece, path)
		return arrange(piece, notes, bar, bpm)
	notes, bar, bpm, dynamics = load_musicxml(piece, path)
	return arrange(piece, notes, bar, bpm, dynamics)


def identifier(title):
	"""Returns a C++ identifier for a piece."""
	return "".join(c if c.isalnum() else "_" for c in title).upper()


def main():
	"""Writes the C++ file with every piece."""
	ap = argparse.ArgumentParser()
	ap.add_argument("--cache", default=os.path.join(tempfile.gettempdir(), "snake-game-midi"))
	ap.add_argument("--out", default=os.path.join(os.path.dirname(__file__), "..", "src", "audio", "score_data.cpp"))
	args = ap.parse_args()
	os.makedirs(args.cache, exist_ok=True)

	lines = ["/**", " * score_data.cpp", " * Author: Miguel Mochizuki Silva",
			 " * Description: The soundtrack. Generated by tools/gen_score.py, do not edit by hand.",
			 " *", " * Sources of the notes:", " *"]
	for p in PIECES:
		lines.append(" * %s, %s (%s%s)" % (p["title"], p["composer"], p["from"], ", Mutopia %d" % p["mutopia"] if "mutopia" in p else ""))
	lines += [" *", " * " + LICENSES.replace("\n", "\n * ")]
	lines += [" */", '#include "snake_game/audio/score.hpp"', ""]
	table = []
	for p in PIECES:
		notes, end = build(p, args.cache)
		name = identifier(p["title"])
		lines += ["// %s: %d notes, %.0f seconds" % (p["title"], len(notes), end / 1000.0), "static const ScoreNote %s[] = {" % name]
		for i in range(0, len(notes), 5):
			lines.append("\t" + " ".join("{%d, %d, %d, %d, %d}," % n for n in notes[i:i + 5]))
		lines += ["};", ""]
		table.append('\t{"%s", "%s", %s, %d, %d, %s},' % (p["title"], p["composer"], name, len(notes), end, "true" if p.get("rare") else "false"))
		print("%-42s %6d notes %6.0f s%s" % (p["title"], len(notes), end / 1000.0, "  (easter egg)" if p.get("rare") else ""))
	lines += ["const Piece PIECES[] = {"] + table + ["};", "", "const int PIECE_COUNT = %d;" % len(PIECES)]
	open(args.out, "w").write("\n".join(lines) + "\n")


if __name__ == "__main__":
	main()
