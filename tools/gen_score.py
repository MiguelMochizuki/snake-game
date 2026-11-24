"""
gen_score.py
Author: Miguel Mochizuki Silva
Description: Writes src/audio/score_data.cpp (the game's soundtrack) from public domain MIDI files of the Mutopia Project.
Every piece becomes a list of notes (start and length in milliseconds, MIDI pitch, melody or accompaniment).
Usage: python3 tools/gen_score.py [--cache DIR] [--out src/audio/score_data.cpp]
The MIDI files are downloaded into the cache folder (default: a temporary folder) when they are not there yet.
"""
import argparse
import os
import struct
import tempfile
import urllib.request

MUTOPIA = "https://www.mutopiaproject.org/ftp/"

# title, composer, Mutopia id, MIDI path under MUTOPIA, source edition, options
#   melody: tracks whose highest note at any time is the melody (the rest of them is accompaniment)
#   bpm: tempo that replaces the one in the file, which Mutopia sets to a placeholder for some pieces
#   repeat: how many times the notes are played one after the other
#   cap_ms / last_ms: Ave Maria only, see convert()
PIECES = [
    {"title": "Ave Maria", "composer": "C. Gounod, over J. S. Bach's Prelude in C major", "id": 2167,
     "path": "GounodC/AveMaria-PianoViolin/AveMaria-PianoViolin.mid", "source": "Heugel, 1856",
     "melody": [1], "accompaniment": [2, 3], "cap_ms": 227, "last_ms": 3636},
    {"title": "Gymnopedie No. 1", "composer": "E. Satie", "id": 37,
     "path": "SatieE/gymnopedie_1/gymnopedie_1.mid", "source": "Dover edition", "melody": [1]},
    {"title": "Ode to Joy", "composer": "L. van Beethoven", "id": 528,
     "path": "BeethovenLv/ode/ode.mid", "source": "various", "melody": [1], "repeat": 2},
    {"title": "Fur Elise", "composer": "L. van Beethoven", "id": 931,
     "path": "BeethovenLv/WoO59/fur_Elise_WoO59/fur_Elise_WoO59.mid", "source": "Breitkopf & Haertel, 1888", "melody": [1]},
    {"title": "Clair de Lune", "composer": "C. Debussy", "id": 1778,
     "path": "DebussyC/L75/debussy_Ste_Bergamesq_Clair/debussy_Ste_Bergamesq_Clair.mid", "source": "E. Fromont, 1905", "melody": [1]},
    {"title": "Rondo alla Turca", "composer": "W. A. Mozart", "id": 108,
     "path": "MozartWA/KV331/KV331_3_RondoAllaTurca/KV331_3_RondoAllaTurca.mid", "source": "IMSLP", "melody": [1], "bpm": 120},
    {"title": "Air on the G String", "composer": "J. S. Bach", "id": 1534,
     "path": "BachJS/BWV1068/bach_air_bmv_1068/bach_air_bmv_1068.mid", "source": "Bach-Gesellschaft", "melody": [1]},
    {"title": "Eine kleine Nachtmusik, 1st movement", "composer": "W. A. Mozart", "id": 900,
     "path": "MozartWA/KV525/eine-kleine-nachtmusik-mvt1/eine-kleine-nachtmusik-mvt1.mid", "source": "IMSLP", "melody": [1]},
    {"title": "Premiere Arabesque", "composer": "C. Debussy", "id": 1777,
     "path": "DebussyC/L66/debussy_Arabesque_1/debussy_Arabesque_1.mid", "source": "Durand et Fils, 1904", "melody": [1]},
    {"title": "In the Hall of the Mountain King", "composer": "E. Grieg", "id": 1888,
     "path": "GriegE/O46/Dans_l_antre_du_roi_de_la_montagne/Dans_l_antre_du_roi_de_la_montagne.mid", "source": "The University Society, 1918", "melody": [1]},
]


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


def notes_of(track):
    """Returns the notes of a track as sorted (start tick, end tick, pitch)."""
    on, notes = {}, []
    for tick, kind, args in track:
        if kind == 0x90 and args[1] > 0:
            on[args[0]] = tick
        elif kind == 0x80 or (kind == 0x90 and args[1] == 0):
            if args[0] in on:
                notes.append((on.pop(args[0]), tick, args[0]))
    return sorted(notes)


def tick_to_ms(division, tempos, tick):
    """Converts a tick to milliseconds following the tempo changes (a missing one means 120 BPM)."""
    if not tempos or tempos[0][0] > 0:
        tempos = [(0, 500000)] + list(tempos)
    ms, last_tick, last_tempo = 0.0, 0, tempos[0][1]
    for t, tempo in tempos[1:]:
        if t >= tick:
            break
        ms += (t - last_tick) * last_tempo / division / 1000.0
        last_tick, last_tempo = t, tempo
    return ms + (tick - last_tick) * last_tempo / division / 1000.0


def skyline(notes):
    """Splits notes into (melody, rest): a note is melody when no note sounding at its start is higher."""
    melody, rest = [], []
    for s, e, p in notes:
        higher = any(s2 <= s < e2 and p2 > p for s2, e2, p2 in notes)
        (rest if higher else melody).append((s, e, p))
    return melody, rest


def convert(piece, path):
    """Returns the piece as a sorted list of (start ms, length ms, pitch, is melody) and its end in ms."""
    division, tempos, tracks = parse_midi(path)
    if "bpm" in piece:
        tempos = [(0, int(60000000 / piece["bpm"]))]

    def ms(tick):
        return tick_to_ms(division, tempos, tick)

    melody_notes, rest_notes = [], []
    for k in piece["melody"]:
        m, r = skyline(notes_of(tracks[k]))
        melody_notes += m
        rest_notes += r
    for k in piece.get("accompaniment", [k for k in range(1, len(tracks)) if k not in piece["melody"]]):
        rest_notes += notes_of(tracks[k])

    notes = []
    for s, e, p in melody_notes:
        notes.append((ms(s), max(30, ms(e) - ms(s)), p, 1))
    last_start = max(s for s, _, _ in rest_notes)
    for s, e, p in rest_notes:
        length = max(30, ms(e) - ms(s))
        if "cap_ms" in piece:
            # Ave Maria: the prelude is an arpeggio of one note per sixteenth; the last chord rings out
            length = piece["last_ms"] if s == last_start else min(length, piece["cap_ms"])
        notes.append((ms(s), length, p, 0))
    notes = sorted((round(a), round(b), p, v) for a, b, p, v in notes)
    end = max(a + b for a, b, _, _ in notes)
    repeated = []
    for n in range(piece.get("repeat", 1)):
        repeated += [(a + n * end, b, p, v) for a, b, p, v in notes]
    return repeated, end * piece.get("repeat", 1)


def fetch(piece, cache):
    """Returns the path of the piece's MIDI file in the cache, downloading it first when needed."""
    path = os.path.join(cache, os.path.basename(piece["path"]))
    if not os.path.exists(path):
        urllib.request.urlretrieve(MUTOPIA + piece["path"], path)
    return path


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

    lines = [
        "/**",
        " * score_data.cpp",
        " * Author: Miguel Mochizuki Silva",
        " * Description: The soundtrack. Generated by tools/gen_score.py, do not edit by hand.",
        " *",
        " * Every piece is in the public domain. The notes come from the Mutopia Project editions listed below,",
        " * through their MIDI files (https://www.mutopiaproject.org/cgibin/piece-info.cgi?id=<number>):",
        " *",
    ]
    for p in PIECES:
        lines.append(" * %s, %s (Mutopia %d, from %s)" % (p["title"], p["composer"], p["id"], p["source"]))
    lines += [" */", '#include "snake_game/audio/score.hpp"', ""]
    table = []
    for p in PIECES:
        notes, end = convert(p, fetch(p, args.cache))
        name = identifier(p["title"])
        lines.append("// %s: %d notes, %.0f seconds" % (p["title"], len(notes), end / 1000.0))
        lines.append("static const ScoreNote %s[] = {" % name)
        for i in range(0, len(notes), 6):
            lines.append("\t" + " ".join("{%d, %d, %d, %d}," % n for n in notes[i:i + 6]))
        lines += ["};", ""]
        table.append('\t{"%s", "%s", %s, %d, %d},' % (p["title"], p["composer"].replace('"', '\\"'), name, len(notes), end))
        print("%-40s %6d notes %6.0f s" % (p["title"], len(notes), end / 1000.0))
    lines += ["const Piece PIECES[] = {"] + table + ["};", "", "const int PIECE_COUNT = %d;" % len(PIECES)]
    open(args.out, "w").write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
