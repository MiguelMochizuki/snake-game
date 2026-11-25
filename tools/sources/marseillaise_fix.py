"""
marseillaise_fix.py
Author: Miguel Mochizuki Silva
Description: La Marseillaise: the Online Sequencer performance (marseillaise_onlinesequencer.mid, a sequence shared on
onlinesequencer.net, license not confirmed) rewritten by hand, bar by bar.
Usage: python3 tools/sources/marseillaise_fix.py [--check]

Writes marseillaise_corrected.mid (this folder): format 1, 384 ticks per quarter, 116 BPM, 4/4, track 0 = melody (square
wave), track 1 = accompaniment (triangle). Run with --check to compare it with the original file and the score.

Time is counted in the bars of the music: the original file starts 2 beats early (its first full beat, G4, sits
at beat 2 of its bar 1), so every original time is shifted by +2 beats. Bar 1 is the pickup bar (D4 on beat 4).
Original file bar n therefore holds the 2nd half of real bar n and the 1st half of real bar n+1.

Conventions of the accompaniment (the consistency rules):
  bass     B(beat, root): the root in two octaves, always the pair (X, X+12) with X between G1 and F#2.
		   Default length 0.75 on quarters, 0.5 before a sixteenth pickup, 0.25 for the pickup itself.
  chord    C(beat, "notes", length): every voice of a chord has the same length; default 0.5 (march staccato),
		   0.75 in the legato bars 12-18, held chords say so explicitly.
  fanfare  F(beat): the D4+D5 octave hits of "Aux armes" (bars 21 and 23), 0.25 each.
  unison   bars 9-11: the melody is doubled 1, 2 and 3 octaves below with its own lengths.
  octave   from bar 23 beat 4 the tune is in octaves: the lower octave (melody - 12) goes in the accompaniment.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import gen_score as g  # noqa: E402

OUT = os.path.join(HERE, "marseillaise_corrected.mid")
DIV, BPM = 384, 116
STEPS = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}


def pitch_of(name):
	"""MIDI note of a name such as C4, F#5 or Bb3."""
	m = re.fullmatch(r"([A-G])([#b]?)(-?\d)", name)
	return 12 * (int(m.group(3)) + 1) + STEPS[m.group(1)] + {"#": 1, "b": -1, "": 0}[m.group(2)]


# The melody of a public domain score of the anthem (in G major, 4/4, from the pickup), as "pitch:beats", r a rest;
# --check compares the tune with it
REFERENCE = (
	"r:2.75 D4:0.25 D4:0.75 D4:0.25  G4:1 G4:1 A4:1 A4:1  D5:1.5 B4:0.5 G4:0.5 r:0.25 G4:0.25 B4:0.75 G4:0.25 "
	"E4:1 C5:2 A4:0.75 F#4:0.25  G4:2 r:1 G4:0.75 A4:0.25  B4:1 B4:1 B4:1 C5:0.75 B4:0.25 "
	"B4:1 A4:1 r:1 A4:0.75 B4:0.25  C5:1 C5:1 C5:1 D5:0.75 C5:0.25  B4:2 r:1 D5:0.75 D5:0.25 "
	"D5:1 B4:0.75 G4:0.25 D5:1 B4:0.75 G4:0.25  D4:2 r:0.75 D4:0.25 D4:0.75 F#4:0.25  A4:2 C5:1 A4:0.75 F#4:0.25 "
	"A4:1 G4:1 F4:2  E4:1 G4:0.75 G4:0.25 G4:1 F#4:0.75 G4:0.25  A4:3 r:0.5 A4:0.5 "
	"Bb4:1.5 Bb4:0.5 Bb4:0.5 Bb4:0.5 C5:0.5 D5:0.5  A4:3 Bb4:0.5 A4:0.5  G4:1.5 G4:0.5 G4:0.5 Bb4:0.5 A4:0.5 G4:0.5 "
	"G4:1 F#4:0.5 r:2.25 D5:0.25  D5:2.75 D5:0.25 B4:0.75 G4:0.25  A4:3 r:0.75 D5:0.25  D5:2.75 D5:0.25 B4:0.75 G4:0.25 "
	"A4:2.5 r:0.5 D4:1  G4:2 r:1 A4:1  B4:2 r:2  C5:2 D5:1 E5:1  A4:2.5 r:0.5 E5:1  D5:2.75 B4:0.25 C5:0.75 A4:0.25 G4:4")


def reference():
	"""[(start, length, pitch)] of REFERENCE."""
	out, t = [], 0.0
	for token in REFERENCE.split():
		name, length = token.split(":")
		if name != "r":
			out.append((t, float(length), pitch_of(name)))
		t += float(length)
	return out

# The tune, as "beat pitch length" per bar. It is the top line of the file, snapped to the sixteenth grid, with
# the notes the recording lost put back (marked R: a repeated pitch whose note-on/note-off pairing broke, leaving
# a ghost of 0.01-0.04 beats or nothing; pitch and rhythm from the public domain score in gen_score.marseillaise()).
MELODY = {
	1: "3 D4 .75 (R) | 3.75 D4 .25",
	2: "0 G4 .75 | 1 G4 .5 | 2 A4 .75 | 3 A4 .75",
	3: "0 D5 1.5 | 1.5 B4 .5 | 2 G4 .5 | 2.75 G4 .25 | 3 B4 .5 | 3.75 G4 .25",
	4: "0 E4 .75 | 1 C5 1.75 | 3 A4 .5 | 3.75 F#4 .25",
	5: "0 G4 2.25 | 3 G4 .5 | 3.75 A4 .25",
	6: "0 B4 1 | 1 B4 1 | 2 B4 .75 | 3 C5 .5 | 3.75 B4 .25 (R)",
	7: "0 B4 1 (R) | 1 A4 1.5 | 3 A4 .5 | 3.75 B4 .25",
	8: "0 C5 1 | 1 C5 .75 | 2 C5 .75 | 3 D5 .5 | 3.75 C5 .25",
	9: "0 B4 2.25 | 3 D5 .5 | 3.75 D5 .25 (R)",
	10: "0 D5 1 | 1 B4 .5 | 1.75 G4 .25 | 2 D5 .75 | 3 B4 .5 | 3.75 G4 .25",
	11: "0 D4 2 | 2.75 D4 .25 | 3 D4 .75 | 3.75 F#4 .25",
	12: "0 A4 2 | 2 C5 1 | 3 A4 .75 | 3.75 F#4 .25",
	13: "0 A4 1 | 1 G4 1 | 2 F4 2",
	14: "0 E4 1 | 1 G4 .5 | 1.75 G4 .25 (R) | 2 G4 1 (R) | 3 F#4 .5 | 3.75 G4 .25",
	15: "0 A4 3 | 3.5 A4 .5",
	16: "0 Bb4 1.25 | 1.5 Bb4 .5 (R) | 2 Bb4 .5 (R) | 2.5 Bb4 .5 | 3 C5 .5 | 3.5 D5 .5",
	17: "0 A4 3 | 3 Bb4 .5 | 3.5 A4 .5",
	18: "0 G4 1.25 | 1.5 G4 .25 | 2 G4 .5 | 2.5 Bb4 .5 | 3 A4 .5 | 3.5 G4 .5",
	19: "0 G4 1 | 1 F#4 1.25 | 3.75 D5 .25 (R)",
	20: "0 D5 2.5 | 2.75 D5 .25 | 3 B4 .5 | 3.75 G4 .25",
	21: "0 A4 3.25 | 3.75 D5 .25 (R)",
	22: "0 D5 2.5 | 2.75 D5 .25 | 3 B4 .5 | 3.75 G4 .25",
	23: "0 A4 2.5 | 3 D5 .75",
	24: "0 G5 2.75 | 3 A5 .75",
	25: "0 B5 4",
	26: "0 C6 2 | 2 D6 .75 | 3 E6 1",
	27: "0 A5 2.75 | 3 E6 1",
	28: "0 D6 2.5 | 2.75 B5 .25 | 3 C6 .5 | 3.75 A5 .25",
	29: "0 G5 4",
}
UNISON = (9 * 4 - 4 + 3, 10 * 4 + 2)  # from bar 9 beat 4 to the end of the D4 of bar 11, in beats
OCTAVES_FROM = 23 * 4 - 4 + 3          # bar 23 beat 4 ("Mar-chons")


def B(beat, root, length=None):
	return ("B", beat, root, length)


def C(beat, notes, length=None):
	return ("C", beat, notes, length)


def F(beat):
	return ("C", beat, "D4 D5", 0.25)


def each(beats, make):
	return [make(b) for b in beats]


L = 0.75  # chord length of the legato bars
G_LH, D_LH = "D3 G3 B3", "D3 F#3 A3"  # the left-hand chords of the oom-pah bars

# The accompaniment, bar by bar. The comment says what changed from the file (see the report for the reasons).
ACCOMPANIMENT = {
	1: [B(3, "D")],
	2: [B(0, "G"), C(0, "B3 D4"), B(1, "B"), C(1, "B3 D4"),               # G G/B D D7/C; B3 0.25 -> 0.5
		B(2, "D"), C(2, "D4 F#4"), B(3, "C"), C(3, "D4 F#4")],
	3: [B(0, "B"), C(0, "D4 G4"), C(1, G_LH + " D4 G4"),                  # LH 0.25 -> 0.5, joined to the RH
		C(2, G_LH + " D4"), B(3, "B"), C(3, "D4 G4")],                    # beat 2 was early (1.86); G4 added on 4
	4: [B(0, "C"), C(0, "G3 C4"), B(1, "A", 1.75), C(1, "E4 A4", 1.75),   # C Am D; G3 added under E4
		B(3, "D"), C(3, "D4 F#4")],
	5: [B(0, "G"), C(0, "B3 D4"), B(1, "D"), C(1, "B3 D4"), B(1.75, "B"),  # G2/G3 -> G1/G2
		B(2, "G"), C(2, "B3 D4"), B(3, "B"), C(3, "B3 D4"),               # B3 added on 4
		B(3.75, "A"), C(3.75, "D4 F#4", 0.25)],                           # D4 added under A4
	6: [B(0, "G"), C(0, "D4 G4"), C(1, G_LH + " D4 G4"),
		B(2, "G"), C(2, "D4 G4"), C(3, G_LH + " D4 G4")],                 # LH on 4 was a lone G3
	7: [B(0, "D"), C(0, "D4 G4"), B(1, "D"), C(1, "D4 F#4"),              # G/D D D Am/C E/B
		B(2, "D"), C(2, "D4 F#4"),                                        # chord missing on 3
		B(3, "C"), C(3, "C4 E4"), B(3.75, "B"), C(3.75, "E4 G#4", 0.25)],  # C4 and E4 added
	8: [B(0, "A"), C(0, "E4 A4"), B(1, "A"), C(1, "E4 A4"),
		B(2, "D"), C(2, "D4 F#4"), B(3, "D"), C(3, "D4 F#4")],
	9: [B(0, "G"), C(0, "D4 G4", 2.25), C(1, G_LH), C(2, G_LH)],          # G2/G3 -> G1/G2; then the unison
	10: [],                                                               # all unison
	11: [B(3, "D")],                                                      # unison D1-D4 on 1, generated
	12: [B(0, "C", 1.75), C(0, "D4 F#4", 1.75), B(2, "C", 1.75),          # D7/C; bass was 0.2 late
		 C(2, "D4 F#4", L), C(3, "D4 F#4", L)],                           # F#4 added on 4
	13: [B(0, "B"), C(0, "A3 D4", L), B(1, "G"), C(1, "G3 B3", L),        # Bm7 G F/A Bdim
		 B(2, "A"), C(2, "A3 C4", L), B(3, "B"), C(3, "B3 D4", L)],       # A3 and B3 added
	14: [B(0, "C"), C(0, "G3 C4", L), B(1, "C"), C(1, "C4 E4", L),        # C C Cm/Eb Cm/Eb (iv6, kept)
		 B(2, "Eb"), C(2, "G3 C4", L), B(3, "Eb"), C(3, "G3 C4", L)],
	15: each([0, 1, 2, 3], lambda b: B(b, "D")) + each([0, 1, 2, 3], lambda b: C(b, "D4 F#4", L)),  # F#4 added
	16: each([0, 1, 2, 3], lambda b: B(b, "D")) + each([0, 1, 2, 3], lambda b: C(b, "D4 G4", L)),   # Gm/D
	17: each([0, 1, 2, 3], lambda b: B(b, "D")) + each([0, 1, 2, 3], lambda b: C(b, "D4 F#4", L)),  # D
	18: [B(0, "Bb"), B(1, "Bb"), B(2, "G"), B(3, "Bb")]                   # Gm/Bb; G3 added on 3 and 4
		+ each([0, 1, 2, 3], lambda b: C(b, "G3 D4", L)),
	19: [B(0, "D"), C(0, "A3 D4", 2), B(1, "D"), B(2, "D", 1.5)],         # Dsus4-D; D3, D2, D1+D2 -> pairs
	20: [B(0, "G"), C(0, "D4 G4 B4", 2.5), C(1, G_LH), C(2, G_LH), C(3, G_LH),  # "Aux armes"
		 C(2.75, "B4", 0.25), C(3, "G4", 0.5), C(3.75, "D4", 0.25)],      # RH sixths; D4 added as in bar 22
	21: [B(0, "D"), C(0, "D4 F#4"), C(1, D_LH), C(2, D_LH), B(3, "D")]    # F#4 added (as bar 23)
		+ each([1, 1.5, 2, 3, 3.5], F),                                   # 3.5 had lost its D5
	22: [B(0, "G"), C(0, "D4 G4 B4", 2.5), C(1, G_LH), C(2, G_LH), C(3, G_LH),
		 C(2.75, "B4", 0.25), C(3, "G4", 0.5), C(3.75, "D4", 0.25)],
	23: [B(0, "D"), C(0, "D4 F#4"), B(1, "C"), B(2, "B"), C(2, "B3 D4 G4 D5", 0.75),  # D D7/C G/B D/A
		 B(3, "A"), C(3, "F#4 A4", 0.75)] + each([1, 1.5, 1.75], F),      # the triplet -> 8th + two 16ths
	24: [B(0, "G"), C(0, "B4 D5", 0.75), B(1, "D"), B(1.75, "D"), B(2, "G"),
		 C(2, "B4 D5", 0.75), B(3, "F#"), C(3, "D5 F#5", 0.75)],
	25: [B(0, "F"), B(1, "F"), B(1.75, "F"), B(2, "F"), B(3, "F"),        # G7/F: the bass descends G F# F E
		 C(0, "D5 G5", 4)],                                               # F3 restored on 2 and 3
	26: [B(0, "E", 1.75), C(0, "E5 G5", 1.75), B(2, "D"), C(2, "G5 B5", 0.75),  # C/E G/D C
		 B(3, "C"), C(3, "C5 G5", 0.75)],                                 # C5 added on 4
	27: [B(0, "D"), C(0, "D5 F#5", 2.75), C(1, D_LH), C(2, D_LH),         # D D7/C
		 B(3, "C"), C(3, "F#5 A5", 0.75)],
	28: [B(0, "B"), C(0, "G5 B5", 2.5), C(1, "D3", 0.5), C(1.75, "G3", 0.25), C(2, "B3", 0.75),  # G/B; G5 added
		 C(2.75, "G5", 0.25), B(3, "D"), C(3, "D5 F#5 A5", 0.5)],         # D7; was 0.15-0.2 late
	29: [B(0, "G"), C(0, "B4 D5", 0.75), B(1, "D"), C(1, "B4 D5", 0.75), B(1.75, "B"),
		 B(2, "G", 2), C(2, "B4 D5", 2)],                                 # ghost chord on 3 restored, held
}

BASS_LOW = pitch_of("G1")  # bass roots go from G1 to F#2, doubled an octave up


def melody():
	"""[(start, length, pitch)] of the tune."""
	out = []
	for bar, text in MELODY.items():
		for item in text.split("|"):
			beat, name, length = item.split()[:3]
			out.append(((bar - 1) * 4 + float(beat), float(length), pitch_of(name)))
	return sorted(out)


def accompaniment(tune):
	"""[(start, length, pitch)] of the accompaniment, with the default lengths filled in."""
	out = []
	for bar, events in ACCOMPANIMENT.items():
		for kind, beat, what, length in events:
			t = (bar - 1) * 4 + beat
			if kind == "B":
				if length is None:
					nxt = min([(b2 - 1) * 4 + e[1] for b2, ev in ACCOMPANIMENT.items() for e in ev
							   if e[0] == "B" and (b2 - 1) * 4 + e[1] > t] + [t + 4])
					gap = nxt - t
					length = min(0.75, gap - 0.25 if gap >= 0.75 else gap)
				root = BASS_LOW + (pitch_of(what + "4") - BASS_LOW) % 12
				out += [(t, length, root), (t, length, root + 12)]
			else:
				out += [(t, length if length is not None else 0.5, pitch_of(n)) for n in what.split()]
	for s, l, p in tune:
		if UNISON[0] <= s < UNISON[1]:
			out += [(s, l, p - 12 * k) for k in (1, 2, 3)]
		if s >= OCTAVES_FROM:
			out.append((s, l, p - 12))
	return sorted(out)


def vlq(n):
	b = [n & 0x7F]
	while n > 127:
		n >>= 7
		b.insert(0, (n & 0x7F) | 0x80)
	return bytes(b)


def track(notes, channel, meta=b""):
	events = []
	for s, l, p in notes:
		events.append((round(s * DIV), 1, bytes([0x90 | channel, p, 90])))
		events.append((round((s + l) * DIV), 0, bytes([0x80 | channel, p, 0])))
	data, now = meta, 0
	for tick, _, msg in sorted(events):
		data += vlq(tick - now) + msg
		now = tick
	data += b"\x00\xff\x2f\x00"
	return b"MTrk" + struct.pack(">I", len(data)) + data


def write(tune, acc):
	tempo = 60000000 // BPM
	meta = (b"\x00\xff\x51\x03" + tempo.to_bytes(3, "big") + b"\x00\xff\x58\x04\x04\x02\x18\x08"
			+ b"\x00\xff\x03\x06Melody")
	with open(OUT, "wb") as f:
		f.write(b"MThd" + struct.pack(">IHHH", 6, 1, 2, DIV))
		f.write(track(tune, 0, meta))
		f.write(track(acc, 1, b"\x00\xff\x03\x0dAccompaniment"))


def check(tune, acc):
	"""Asserts the rules; prints how the tune differs from the file's top line and from the score."""
	allnotes = [(s, l, p, "m") for s, l, p in tune] + [(s, l, p, "a") for s, l, p in acc]
	for s, l, p, _ in allnotes:
		assert s * 4 == int(s * 4) and l * 4 == int(l * 4) and l >= 0.25, (s, l, p)
	for i, a in enumerate(allnotes):  # no pitch struck again while it still sounds
		for b in allnotes[i + 1:]:
			assert not (a[2] == b[2] and a[0] < b[0] + b[1] - 1e-9 and b[0] < a[0] + a[1] - 1e-9), (a, b)
	peak = max(sum(1 for s, l, *_ in allnotes if s <= t / 4 < s + l) for t in range(4 * 120))
	assert peak <= 12, peak
	# The tune is the top line, except under the fanfare of bars 21 and 23 (beats 80-92), as in the file
	above = [(s, p) for s, l, p in tune for o in acc if o[0] <= s < o[0] + o[1] and o[2] > p and not 80 <= s < 92]
	above += [(o[0], o[2]) for o in acc for s, l, p in tune if s <= o[0] < s + l and o[2] > p and not 80 <= o[0] < 92]
	assert not above, above
	print(f"ok: grid, no doubled pitches, at most {peak} voices at once, {len(tune)} + {len(acc)} notes")


def compare(tune):
	"""Prints the differences of the tune from the file's (raw) top line and from the score's melody."""
	N = "C C# D D# E F F# G G# A A# B".split()
	nm = lambda p: N[p % 12] + str(p // 12 - 1)
	div, _, tracks = g.parse_midi(os.path.join(HERE, "marseillaise_onlinesequencer.mid"))
	raw = [(s / div + 2, (e - s) / div, p) for s, e, p, v in g.midi_notes(tracks[0])]
	ref = sorted((s, l, p + (12 if s >= OCTAVES_FROM else 0)) for s, l, p in reference())
	print("tune vs file (raw onset within 0.3, same pitch) and vs score:")
	for s, l, p in tune:
		c = sorted((n for n in raw if n[2] == p and abs(n[0] - s) < 0.3), key=lambda n: abs(n[0] - s))
		f = f"file {c[0][0] - s:+.2f} len {c[0][1]:.2f}" if c else "file: none"
		r = [x for x in ref if x[0] == s and x[2] == p]
		rs = f"score len {r[0][1]}" if r else "score: none"
		bad = not c or c[0][1] < 0.1 or abs(c[0][0] - s) > 0.13 or abs(round(c[0][1] * 4) / 4 - l) > 0.01
		if bad or not r or r[0][1] != l:
			print(f"  bar {int(s // 4) + 1:2d} beat {s % 4 + 1:5.2f} {nm(p):4} len {l:<5} {f:24} {rs}")
	missing = [x for x in ref if not any(t[0] == x[0] and t[2] == x[2] for t in tune)]
	print("score notes not in the tune:", [(int(s // 4) + 1, s % 4 + 1, nm(p)) for s, l, p in missing])


if __name__ == "__main__":
	tune = melody()
	acc = accompaniment(tune)
	check(tune, acc)
	write(tune, acc)
	print("wrote", OUT)
	if "--check" in sys.argv:
		compare(tune)
