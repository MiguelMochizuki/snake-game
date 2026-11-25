"""
fix_omr.py
Author: Miguel Mochizuki Silva
Description: Corrects the optical music recognition (Audiveris) of the two Musica Brasilis scores against their PDFs and
writes the files tools/gen_score.py reads:
	nazareth_omr.mxl -> nazareth.mxl   (Apanhei-te, cavaquinho, E. Nazareth, IMSLP 614247)
	tico_omr.mxl     -> tico.mxl       (Tico-tico no fuba, Zequinha de Abreu, IMSLP 1040456)
Both editions are by Musica Brasilis under CC BY-SA 4.0, and so are these files and the notes made from them.
Every correction is listed below, by printed bar number. The measures are renumbered so that the "number" of each
measure is the printed bar number (the pickup of Apanhei-te is bar 0), which is what gen_score.py's "order" uses.
Usage: python3 tools/sources/fix_omr.py
"""
import copy
import io
import os
import re
import xml.etree.ElementTree as ET
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
DIVISIONS = 4  # both files count in sixteenths
# Apanhei-te is written for the right hand an octave below its sound (8va) from start to end, which puts the tune up
# to A7 (3.5 kHz). False plays it as written, an octave lower, should the square wave be too shrill up there
EIGHT_VA = True
LENGTHS = {"s": 1, "e": 2, "e.": 3, "q": 4, "q.": 6, "h": 8}
TYPES = {1: ("16th", False), 2: ("eighth", False), 3: ("eighth", True), 4: ("quarter", False), 6: ("quarter", True),
		 8: ("half", False)}


def load(name):
	"""Returns the score element of a compressed MusicXML file of this folder."""
	z = zipfile.ZipFile(os.path.join(HERE, name))
	return ET.fromstring(z.read([n for n in z.namelist() if n.endswith(".xml") and not n.startswith("META")][0]))


def save(root, name):
	"""Writes the score as a compressed MusicXML file of this folder."""
	data = io.BytesIO()
	ET.ElementTree(root).write(data, encoding="UTF-8", xml_declaration=True)
	container = ('<?xml version="1.0" encoding="UTF-8"?><container><rootfiles><rootfile full-path="score.xml"/>'
				 '</rootfiles></container>')
	with zipfile.ZipFile(os.path.join(HERE, name), "w", zipfile.ZIP_DEFLATED) as z:
		z.writestr(zipfile.ZipInfo("META-INF/container.xml", (2026, 1, 1, 0, 0, 0)), container)
		z.writestr(zipfile.ZipInfo("score.xml", (2026, 1, 1, 0, 0, 0)), data.getvalue())


def note(pitch, length, staff, voice, chord, ties=()):
	"""A <note> element: pitch is a name such as F#5 or Bb4, or "r" for a rest; length in sixteenths; ties: "start"
	and/or "stop"."""
	n = ET.Element("note")
	if chord:
		ET.SubElement(n, "chord")
	if pitch == "r":
		ET.SubElement(n, "rest")
	else:
		m = re.fullmatch(r"([A-G])([#b]?)(\d)", pitch)
		p = ET.SubElement(n, "pitch")
		ET.SubElement(p, "step").text = m.group(1)
		if m.group(2):
			ET.SubElement(p, "alter").text = "1" if m.group(2) == "#" else "-1"
		ET.SubElement(p, "octave").text = m.group(3)
	ET.SubElement(n, "duration").text = str(length)
	for t in ties:
		ET.SubElement(n, "tie").set("type", t)
	ET.SubElement(n, "voice").text = voice
	kind, dot = TYPES[length]
	ET.SubElement(n, "type").text = kind
	if dot:
		ET.SubElement(n, "dot")
	ET.SubElement(n, "staff").text = staff
	return n


def notes(text, staff):
	"""Notes of one staff written as "C5+D5+F#5/e. r/e G6/s": chords joined by +, a length after the slash, ~ after it
	ties the notes to the same pitches of the next chord."""
	out, total, voice, tied = [], 0, "1" if staff == "1" else "5", set()
	for token in text.split():
		names, length = token.rstrip("~").split("/")
		for k, name in enumerate(names.split("+")):
			ties = (["stop"] if name in tied else []) + (["start"] if token.endswith("~") else [])
			out.append(note(name, LENGTHS[length], staff, voice, k > 0, ties))
		tied = set(names.split("+")) if token.endswith("~") else set()
		total += LENGTHS[length]
	return out, total


def staff_of(e):
	"""Staff number of a <note> element, as text."""
	return e.findtext("staff") or "1"


def set_staves(measure, **staves):
	"""Rewrites the notes of a measure: staves given as staff1="..." / staff2="..." replace that staff; the others keep
	their notes. Directions, barlines and attributes stay in place."""
	keep = {}
	for s in ("1", "2"):
		old = [e for e in measure if e.tag == "note" and staff_of(e) == s]
		keep[s] = notes(staves["staff" + s], s) if "staff" + s in staves else (
			old, sum(int(e.findtext("duration")) for e in old if e.find("chord") is None and e.find("grace") is None))
	for e in [e for e in measure if e.tag in ("note", "backup", "forward")]:
		measure.remove(e)
	end = [e for e in measure if e.tag == "barline" and e.get("location") == "right"]
	for e in end:
		measure.remove(e)
	measure.extend(keep["1"][0])
	backup = ET.SubElement(measure, "backup")
	ET.SubElement(backup, "duration").text = str(keep["1"][1])
	measure.extend(keep["2"][0])
	measure.extend(end)


def measures(root):
	"""The measures of the (only) part."""
	return root.find("part").findall("measure")


def renumber(root, numbers):
	"""Gives each measure its printed bar number, from a list in order."""
	for m, n in zip(measures(root), numbers):
		m.set("number", str(n))


def delete(root, index):
	"""Removes a measure the recognition made up (an empty one at a double barline)."""
	root.find("part").remove(measures(root)[index])


def whole_8va(root):
	"""The whole right hand of Apanhei-te is under an 8va bracket: the recognition found some of the brackets only (and
	one 8vb in the left hand that is not there), so they are replaced by one bracket from the first note to the end."""
	for m in measures(root):
		for d in [d for d in m if d.tag == "direction" and d.find(".//octave-shift") is not None]:
			m.remove(d)
	if not EIGHT_VA:
		return
	d = ET.Element("direction")
	shift = ET.SubElement(ET.SubElement(d, "direction-type"), "octave-shift")
	shift.set("type", "down")
	shift.set("size", "8")
	ET.SubElement(d, "staff").text = "1"
	first = measures(root)[0]
	first.insert(list(first).index(first.find("note")), d)


def nazareth():
	"""Apanhei-te, cavaquinho."""
	root = load("nazareth_omr.mxl")
	by_index = measures(root)
	# The index of the recognized file (0 is the pickup) is the printed bar number up to bar 51; index 52 is an empty
	# measure at the double barline before the Trio, so index i > 52 is bar i - 1
	bar = {n: by_index[n if n <= 51 else n + 1] for n in range(0, 69)}
	# bar 33 (first ending of the second part): a sixteenth rest was lost before the turn B C B
	set_staves(bar[33], staff1="E6/q r/s B5/s C6/s B5/s")
	# bar 50 ("somente para acabar", Fine): the left hand lost its first chord
	set_staves(bar[50], staff2="B3+D4+G4/q G3+D4+G4/q")
	# bars 53 and 61: the chord rhythm is dotted eighth, sixteenth, eighth rest, eighth (it was read without the rest);
	# bar 61 also lost its left hand, the same as bar 53's
	trio_chord = "C5+D5+F#5+A5"
	set_staves(bar[53], staff1="%s/e. %s/s r/e %s/e" % ((trio_chord,) * 3))
	set_staves(bar[61], staff1="%s/e. %s/s r/e %s/e" % ((trio_chord,) * 3),
			   staff2="C4+D4+F#4/e D4+F#4+A4/s D4+F#4+A4/s D4+F#4+A4/e D4+F#4+A4/e")
	# bars 55, 63 and 65: the same rhythm (55 was split into two voices, 63 and 65 had no rest)
	set_staves(bar[55], staff1="E5+C6+E6/e. E5+C6+E6/s r/e E5+C6+E6/e")
	set_staves(bar[63], staff1="E5+B5+E6/e. E5+B5+E6/s r/e E5+B5+E6/e")
	set_staves(bar[65], staff1="A5+D6+A6/e. A5+D6+A6/s r/e A5+D6+A6/e")
	# bar 64: E6 after the D#6 is a sixteenth, not an eighth (the bar was a sixteenth too long)
	set_staves(bar[64], staff1="r/s D#6/e E6/s F6/s E6/s D#6/s E6/s")
	delete(root, 52)
	renumber(root, range(0, 69))
	whole_8va(root)
	save(root, "nazareth.mxl")


def tico():
	"""Tico-tico no fuba."""
	root = load("tico_omr.mxl")
	m = measures(root)
	# bar 1: a "pp" was read on top of the "f"
	for d in [d for d in m[0] if d.tag == "direction" and d.find(".//pp") is not None]:
		m[0].remove(d)
	# bar 6: the tie of the G#4-E5 chord was lost (bars 5, 13 and 14 have it)
	set_staves(m[5], staff1="F4/s E4/s G#4+E5/e~ G#4+E5/s E4/s D#4/s E4/s")
	# bars 40-42: the change to C major at bar 40 was lost (it is on the empty measure the recognition made at the
	# double barline, index 39), so they were read with three sharps; they are the same as bars 48-50
	for k in (40, 41, 42):
		for e in [e for e in m[k] if e.tag in ("note", "backup", "forward")]:
			m[k].remove(e)
		end = [e for e in m[k] if e.tag == "barline" and e.get("location") == "right"]
		for e in end:
			m[k].remove(e)
		m[k].extend(copy.deepcopy(e) for e in m[k + 8] if e.tag in ("note", "backup", "forward"))
		m[k].extend(end)
	key = ET.Element("attributes")
	ET.SubElement(ET.SubElement(key, "key"), "fifths").text = "0"
	m[40].insert(0, key)
	delete(root, 39)
	renumber(root, range(1, 57))
	save(root, "tico.mxl")


if __name__ == "__main__":
	nazareth()
	tico()
	print("wrote", os.path.join(HERE, "nazareth.mxl"), "and", os.path.join(HERE, "tico.mxl"))
