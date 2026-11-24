"""
gen_score.py
Author: Miguel Mochizuki Silva
Description: Writes src/audio/score_data.cpp (the game's soundtrack). Every piece is a list of notes (start and length in
milliseconds, MIDI pitch, melody or accompaniment, velocity) built from a MIDI file, a MusicXML file or a hand-written
arrangement. An arrangement layer then plays the piece the way it is performed: repeats written out, a tempo, a
ritardando, a dynamics curve and a balance between the accompaniment and the melody.
Usage: python3 tools/gen_score.py [--cache DIR] [--out src/audio/score_data.cpp]
The files are downloaded into the cache folder (default: a temporary folder) when they are not there yet.
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
OPENSCORE = "https://raw.githubusercontent.com/OpenScore/Lieder/main/scores/"
STEPS = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}
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
    """Returns the notes of a MIDI track as sorted (start tick, end tick, pitch, velocity)."""
    on, notes = {}, []
    for tick, kind, args in track:
        if kind == 0x90 and args[1] > 0:
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


def load_midi(spec, path):
    """Reads a MIDI piece: returns (notes, beats per bar, tempo in BPM from the file)."""
    division, tempos, tracks = parse_midi(path)
    sig = re.search(b"\xff\x58\x04(....)", open(path, "rb").read())
    beats_per_bar = sig.group(1)[0] * 4.0 / 2 ** sig.group(1)[1] if sig else 4.0
    melody, rest = [], []
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
        return [n for n in timed if n[0] < cut], 4.0, 60.0
    notes = [(s / division, (e - s) / division, p, v, True) for s, e, p, v in melody]
    notes += [(s / division, (e - s) / division, p, v, False) for s, e, p, v in rest]
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
            midi = 12 * (int(p.find("octave").text) + 1) + STEPS[p.find("step").text]
            if p.find("alter") is not None:
                midi += int(float(p.find("alter").text))
            ties = {t.get("type") for t in e.findall("tie")}
            if "stop" in ties and midi in tied:
                tied[midi]["len"] += duration / state["divisions"]
                if "start" not in ties:
                    del tied[midi]
                continue
            n = {"start": start / state["divisions"], "len": duration / state["divisions"], "pitch": midi}
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
        measures = [read_measure(m, state) for m in part.findall("measure")]
        order = order or unfold_measures(measures)
        beat, points, ramp = 0.0, [], None
        for mi in order:
            m = measures[mi]
            for n in m["notes"]:
                notes.append((beat + n["start"], n["len"], n["pitch"], 0, index == spec["melody"][0], index))
            for at, kind, value in sorted(m["dyn"]):
                if kind == "set":
                    points.append((beat + at, value))
                elif value in ("crescendo", "diminuendo"):
                    ramp = (beat + at, points[-1][1] if points else 64, 22 if value == "crescendo" else -22)
                elif ramp:
                    points += [(ramp[0], ramp[1]), (beat + at, ramp[1] + ramp[2])]
                    ramp = None
            beat += m["length"] or state["bar"]
        dynamics[index] = sorted(points)
    return notes, state["bar"], state["bpm"] or 72.0, dynamics


# Hand-written arrangements: tunes in the public domain with an accompaniment of the game's own.

def pitch_of(name):
    """MIDI note of a name such as C4, F#5 or Bb3."""
    m = re.fullmatch(r"([A-G])([#b]?)(-?\d)", name)
    return 12 * (int(m.group(3)) + 1) + STEPS[m.group(1)] + {"#": 1, "b": -1, "": 0}[m.group(2)]


def line(text):
    """Turns 'F#5:1 E5:0.5 r:1' into [(start, length, pitch)]; r is a rest."""
    out, t = [], 0.0
    for token in text.split():
        name, length = token.split(":")
        if name != "r":
            out.append((t, float(length), pitch_of(name)))
        t += float(length)
    return out


CHORDS = {"D": "D F# A", "A": "A C# E", "G": "G B D", "E": "E G# B", "Am": "A C E", "Dm": "D F A", "C": "C E G",
          "Gm": "G Bb D"}


def triad(chord, octave):
    """The three MIDI notes of a chord, the root in the given octave."""
    root = pitch_of(CHORDS[chord].split()[0] + str(octave))
    return [root] + [(p if p > root else p + 12) for p in (pitch_of(n + str(octave)) for n in CHORDS[chord].split()[1:])]


def oompah(chords, per_bar, start, velocity, stabs=True):
    """Bass and chords in a march pattern: bass on the odd beats, chord on the even ones. chords has `per_bar` per bar."""
    notes, span = [], 4.0 / per_bar
    for i, c in enumerate(chords):
        t = start + i * span
        root, _, fifth = triad(c, 2)
        for b in range(int(span)):
            if b % 2 == 0:
                notes.append((t + b, 0.9, root if b == 0 else fifth, velocity + 12, False))
            elif stabs:
                notes += [(t + b, 0.7, p, velocity, False) for p in triad(c, 3)]
    return notes


def ode_to_joy():
    """The theme of the 4th movement of the 9th symphony in D major, three laps that grow: bass, then chords, then more."""
    a = line("F#5:1 F#5:1 G5:1 A5:1 A5:1 G5:1 F#5:1 E5:1 D5:1 D5:1 E5:1 F#5:1 F#5:1.5 E5:0.5 E5:2")
    a2 = line("F#5:1 F#5:1 G5:1 A5:1 A5:1 G5:1 F#5:1 E5:1 D5:1 D5:1 E5:1 F#5:1 E5:1.5 D5:0.5 D5:2")
    b = line("E5:1 E5:1 F#5:1 D5:1 E5:1 F#5:0.5 G5:0.5 F#5:1 D5:1 E5:1 F#5:0.5 G5:0.5 F#5:1 E5:1 D5:1 E5:1 A4:2")
    chords = ("D D A A D D A A  D D A A D D A D  G G D D A A D A  D D A A D D A D").split()
    notes = []
    for lap in range(3):
        base = lap * 64
        for k, phrase in enumerate((a, a2, b, a2)):
            for t, l, p in phrase:
                notes.append((base + k * 16 + t, l, p, 100, True))
                if lap == 2:
                    notes.append((base + k * 16 + t, l, p - 12, 75, False))
        notes += oompah(chords, 2, base, 52 + 12 * lap, stabs=lap > 0)
        if lap == 2:
            for i, c in enumerate(chords):
                notes += [(base + i * 2 + e * 0.5, 0.5, triad(c, 4)[e % 3], 55, False) for e in range(4)]
    end = 3 * 64
    notes += [(end, 6.0, p, 100, False) for p in triad("D", 3) + [pitch_of("D2"), pitch_of("D4")]]
    notes.append((end, 6.0, pitch_of("D5"), 100, True))
    return notes, 4.0


def korobeiniki():
    """The Russian folk song of the Tetris theme, in A minor: the tune twice, over a march, three times."""
    a = line("E5:1 B4:0.5 C5:0.5 D5:1 C5:0.5 B4:0.5 A4:1 A4:0.5 C5:0.5 E5:1 D5:0.5 C5:0.5 B4:1.5 C5:0.5 D5:1 E5:1 C5:1 A4:1 A4:2")
    b = line("D5:1.5 F5:0.5 A5:1 G5:0.5 F5:0.5 E5:1.5 C5:0.5 E5:1 D5:0.5 C5:0.5 B4:1 B4:0.5 C5:0.5 D5:1 E5:1 C5:1 A4:1 A4:2")
    chords_a = "Am Am Am Am E E Am Am".split()
    chords_b = "Dm Dm C C E E Am Am".split()
    notes, t = [], 0.0
    for lap in range(3):
        for tune, chords in ((a, chords_a), (b, chords_b), (a, chords_a), (b, chords_b)):
            notes += [(t + s, l, p, 100, True) for s, l, p in tune]
            notes += oompah(chords, 2, t, 58 + 8 * lap)
            t += 16.0
    notes += [(t, 4.0, p, 100, False) for p in triad("Am", 3) + [pitch_of("A2")]]
    notes.append((t, 4.0, pitch_of("A4"), 100, True))
    return notes, 4.0


def marseillaise():
    """The melody of La Marseillaise (Rouget de Lisle, in G major), read from a public domain score, over a march."""
    melody = line(
        "r:2.75 D4:0.25 D4:0.75 D4:0.25  G4:1 G4:1 A4:1 A4:1  D5:1.5 B4:0.5 G4:0.5 r:0.25 G4:0.25 B4:0.75 G4:0.25 "
        "E4:1 C5:2 A4:0.75 F#4:0.25  G4:2 r:1 G4:0.75 A4:0.25  B4:1 B4:1 B4:1 C5:0.75 B4:0.25 "
        "B4:1 A4:1 r:1 A4:0.75 B4:0.25  C5:1 C5:1 C5:1 D5:0.75 C5:0.25  B4:2 r:1 D5:0.75 D5:0.25 "
        "D5:1 B4:0.75 G4:0.25 D5:1 B4:0.75 G4:0.25  D4:2 r:0.75 D4:0.25 D4:0.75 F#4:0.25  A4:2 C5:1 A4:0.75 F#4:0.25 "
        "A4:1 G4:1 F4:2  E4:1 G4:0.75 G4:0.25 G4:1 F#4:0.75 G4:0.25  A4:3 r:0.5 A4:0.5 "
        "Bb4:1.5 Bb4:0.5 Bb4:0.5 Bb4:0.5 C5:0.5 D5:0.5  A4:3 Bb4:0.5 A4:0.5  G4:1.5 G4:0.5 G4:0.5 Bb4:0.5 A4:0.5 G4:0.5 "
        "G4:1 F#4:0.5 r:2.25 D5:0.25  D5:2.75 D5:0.25 B4:0.75 G4:0.25  A4:3 r:0.75 D5:0.25  D5:2.75 D5:0.25 B4:0.75 G4:0.25 "
        "A4:2.5 r:0.5 D4:1  G4:2 r:1 A4:1  B4:2 r:2  C5:2 D5:1 E5:1  A4:2.5 r:0.5 E5:1  D5:2.75 B4:0.25 C5:0.75 A4:0.25 G4:4")
    chords = "G G G C G G D C G G D D D C D Gm D Gm D G D G D G G C D D G".split()
    notes = [(s, l, p, 100, True) for s, l, p in melody]
    return notes + oompah(chords, 1, 0.0, 64), 4.0


# The repertoire.
#   source:   "midi" and "musicxml" need a url, "hand" names a function above
#   melody:   MIDI tracks (or the MusicXML part) whose highest note at any time is the melody
#   form:     bars (1-based, inclusive) in the order they are played; the repeats are written out here
#   bpm:      tempo that replaces the one of the file; rit: the last bars slow down to a fraction of the tempo
#   dyn:      (bar of the form, level 0-100) points of the volume curve, joined by straight lines
#   accompaniment_gain: (first bar, last bar, factor) changes of the accompaniment's volume
#   cap/last: Ave Maria's prelude is an arpeggio of one note per sixteenth, with a chord that rings out at the end
#   rare:     an easter egg, chosen with 1% of the probability each

PIECES = [
    {"title": "Ave Maria", "composer": "C. Gounod, over J. S. Bach's Prelude in C major", "mutopia": 2167, "from": "Heugel, 1856",
     "source": "midi", "url": MUTOPIA + "GounodC/AveMaria-PianoViolin/AveMaria-PianoViolin.mid",
     "melody": [1], "accompaniment": [2, 3], "cap": 0.25, "last": 4.0, "bpm": 66, "rit": {"bars": 3, "factor": 0.45},
     "dyn": [(1, 62), (16, 70), (36, 78), (60, 90), (68, 100), (73, 100), (74, 85)],
     "accompaniment_gain": [(66, 74, 1.7)]},
    {"title": "Ode to Joy", "composer": "L. van Beethoven, arranged for the game", "from": "the theme of the 9th symphony, 4th movement",
     "source": "hand", "function": "ode_to_joy", "bpm": 132, "rit": {"bars": 2, "factor": 0.6}},
    {"title": "Fur Elise", "composer": "L. van Beethoven", "from": "Wikimedia Commons, Furelise.mid (public domain)",
     "source": "midi", "url": commons("Furelise.mid"), "melody": [1], "bpm": 80, "rit": {"bars": 3, "factor": 0.6}},
    {"title": "Rondo alla Turca", "composer": "W. A. Mozart", "from": "Wikimedia Commons, Rondo Alla Turka.mid (public domain)",
     "source": "midi", "url": commons("Rondo Alla Turka.mid"), "melody": [1], "bpm": 132, "rit": {"bars": 2, "factor": 0.7}},
    {"title": "Air on the G String", "composer": "J. S. Bach", "mutopia": 1534, "from": "Bach-Gesellschaft",
     "source": "midi", "url": MUTOPIA + "BachJS/BWV1068/bach_air_bmv_1068/bach_air_bmv_1068.mid", "melody": [1],
     "form": [(1, 8), (1, 8), (9, 19), (9, 19)], "bpm": 56, "rit": {"bars": 2, "factor": 0.55},
     "dyn": [(1, 70), (9, 82), (17, 72), (25, 82), (33, 70), (38, 55)]},
    {"title": "Eine kleine Nachtmusik, 1st movement", "composer": "W. A. Mozart", "mutopia": 900, "from": "IMSLP",
     "source": "midi", "url": MUTOPIA + "MozartWA/KV525/eine-kleine-nachtmusik-mvt1/eine-kleine-nachtmusik-mvt1.mid",
     "melody": [1], "bpm": 168, "rit": {"bars": 2, "factor": 0.8}},
    {"title": "Leise flehen meine Lieder (Serenade)", "composer": "F. Schubert", "from": "OpenScore Lieder Corpus (CC0), from IMSLP",
     "source": "musicxml", "url": OPENSCORE + urllib.parse.quote("Schubert,_Franz/Schwanengesang,_D.957/04_Ständchen/lc5004835.mxl"),
     "melody": [0], "bpm": 70, "rit": {"bars": 2, "factor": 0.6}},
    {"title": "Hungarian Rhapsody No. 2", "composer": "F. Liszt", "from": "Wikimedia Commons, Liszt - Hungarian Rhapsody No. 2.mid, performed by B. Krueger (CC BY-SA 3.0 de, piano-midi.de)",
     "source": "midi", "url": commons("Liszt - Hungarian Rhapsody No. 2.mid"), "melody": [1], "performance": True,
     "bpm": 60, "rit": {"bars": 1, "factor": 0.8}},
    {"title": "Korobeiniki (the Tetris theme)", "composer": "traditional Russian folk song", "from": "the traditional tune, arranged for the game",
     "source": "hand", "function": "korobeiniki", "bpm": 156, "rit": {"bars": 2, "factor": 0.6}, "rare": True},
    {"title": "La Marseillaise", "composer": "C. J. Rouget de Lisle", "from": "the melody of a public domain score, with an accompaniment written for the game",
     "source": "hand", "function": "marseillaise", "bpm": 104, "rit": {"bars": 1, "factor": 0.6}, "rare": True,
     "dyn": [(1, 85), (9, 100), (11, 60), (15, 65), (19, 100), (29, 100)]},
]


def fetch(piece, cache):
    """Returns the path of the piece's file in the cache, downloading it first when needed."""
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
            out += [(n[0] - lo + offset,) + n[1:] for n in notes if lo <= n[0] < hi]
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
        v = 100.0 * v / top
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

    final = []
    for s, l, p, v, m in shaped:
        a, b = seconds(s), seconds(s + l)
        final.append((round(a * 1000), max(30, round((b - a) * 1000)), p, 1 if m else 0, v))
    final.sort()
    return final, max(a + b for a, b, _, _, _ in final)


def build(piece, cache):
    """Returns the finished notes and end time of a piece."""
    if piece["source"] == "hand":
        notes, bar = globals()[piece["function"]]()
        return arrange(piece, notes, bar, 120.0)
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
             " *", " * Every piece is in the public domain or CC0, except the Hungarian Rhapsody, a performance by Bernd Krueger under",
             " * CC BY-SA 3.0 de (the notes made from it are under that license). Sources of the notes:", " *"]
    for p in PIECES:
        lines.append(" * %s, %s (%s%s)" % (p["title"], p["composer"], p["from"], ", Mutopia %d" % p["mutopia"] if "mutopia" in p else ""))
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
