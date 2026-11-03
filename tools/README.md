This folder holds maintenance scripts.

`gen_score.py` regenerates `src/audio/score_data.cpp` (the soundtrack) from the Mutopia MIDI named in its header.

`make_icon.py` redraws `assets/icon.{png,ico,icns}` (needs Pillow: `pip install pillow`). Run `python3 tools/make_icon.py` and commit the three files.
