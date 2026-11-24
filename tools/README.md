This folder holds maintenance scripts.

`gen_score.py` regenerates `src/audio/score_data.cpp` (the soundtrack) from MIDI files (Mutopia, Wikimedia Commons), MusicXML files (OpenScore) and hand-written arrangements, listed in its `PIECES` table; it downloads the files into a cache folder (`--cache`) when they are not there. A row sets the tempo, a ritardando, a volume curve, the order of the bars (for repeats) and whether the piece is an easter egg. To add a piece, add a row to the table and run it again.

`icon_main.cpp` is the `snake_icon` tool: it draws the icon artwork with the game's own sprites and writes it as a PPM. It needs no SDL.

`make_icon.py` runs that tool, puts the picture on a rounded square and writes `assets/icon.{png,ico,icns}`, `assets/icon-512.png` and `src/gui/icon_data.cpp` (needs Pillow: `pip install pillow`). Run `make icon`, which builds the tool in `build-icon/` first, and commit the generated files.
