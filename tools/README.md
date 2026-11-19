This folder holds maintenance scripts.

`gen_score.py` regenerates `src/audio/score_data.cpp` (the soundtrack) from the Mutopia MIDI named in its header.

`icon_main.cpp` is the `snake_icon` tool: it draws the icon artwork with the game's own sprites and writes it as a PPM. It needs no SDL.

`make_icon.py` runs that tool, puts the picture on a rounded square and writes `assets/icon.{png,ico,icns}`, `assets/icon-512.png` and `src/gui/icon_data.cpp` (needs Pillow: `pip install pillow`). Run `make icon`, which builds the tool in `build-icon/` first, and commit the generated files.
