This folder holds maintenance scripts.

`gen_score.py` regenerates `src/audio/score_data.cpp` (the soundtrack) from MIDI files (Mutopia, Wikimedia Commons) and MusicXML files, listed in its `PIECES` table; it downloads the files with a stable URL into a cache folder (`--cache`) when they are not there. A row sets the tempo, a ritardando, a volume curve, the order of the bars (for repeats), the balance between melody and accompaniment, the loudness and whether the piece is an easter egg. To add a piece, add a row to the table and run it again, then `make balance` (below) and set its `gain` and `melody_db` until it passes at -20.2 LUFS.

`sources/` holds the files that have no stable URL, so that `gen_score.py` needs nothing else:

- `standchen_midifind.mid`: Schubert's *Ständchen*, a MIDI file of MIDIfind.com (license not confirmed).
- `marseillaise_onlinesequencer.mid`: *La Marseillaise*, a sequence of Online Sequencer (license not confirmed); `marseillaise_fix.py` rewrites it bar by bar into `marseillaise_corrected.mid`, which `gen_score.py` reads (`--check` compares it with the original and with the score).
- `tetris_nonstop2k.mid`: *Korobeiniki*, a chiptune MIDI file of nonstop2k.com (license not confirmed).
- `nazareth_omr.mxl` and `tico_omr.mxl`: *Apanhei-te, cavaquinho* (E. Nazareth) and *Tico-tico no fubá* (Zequinha de Abreu) as Audiveris read the PDFs of the Musica Brasilis editions (IMSLP 614247 and 1040456, CC BY-SA 4.0); `fix_omr.py` corrects them against the PDFs (every correction is listed in it) and writes `nazareth.mxl` and `tico.mxl`, which `gen_score.py` reads. These four files are under CC BY-SA 4.0.

`render_music.cpp` is the `snake_render_music` tool that `make music` builds: it writes every piece as a WAV file, played by the game's synth (or only its melody or its accompaniment).

`check_balance.py` (`make balance`, needs numpy, scipy and ffmpeg) measures every WAV file of `music/`: the sample peak (at most -3 dBFS), the loudness, the melody against the accompaniment (3 to 8 dB, A-weighted, each rendered alone) and the bass and treble against the middle (warnings only). It fails when a piece clips or its melody does not stand out.

`icon_main.cpp` is the `snake_icon` tool: it draws the icon artwork with the game's own sprites and writes it as a PPM. It needs no SDL.

`make_icon.py` runs that tool, puts the picture on a rounded square and writes `assets/icon.{png,ico,icns}`, `assets/icon-512.png` and `src/gui/icon_data.cpp` (needs Pillow: `pip install pillow`). Run `make icon`, which builds the tool in `build-icon/` first, and commit the generated files.
