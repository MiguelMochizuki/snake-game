/**
 * score.hpp
 * Author: Miguel Mochizuki Silva
 * Description: The soundtrack as data: Gounod's "Ave Maria" melody over Bach's C major Prelude
 */
#ifndef SCORE_HPP
#define SCORE_HPP

/**
 * One note of the melody
 *
 * Attributes:
 * int start: First sixteenth-note step of the note
 * int length: Duration in sixteenths
 * int pitch: MIDI note number
 */
struct MelodyNote {
	int start;
	int length;
	int pitch;
};

// 66 BPM (Andante semplice, as marked in the score): a calm tempo that is easy to play over
const double TEMPO_BPM = 66.0;

// Accompaniment: one note per sixteenth, then a final chord, then a rest before the piece restarts
const int ARPEGGIO_STEPS = 1152;
const int SCORE_STEPS = 1184;
const int FINAL_CHORD_SIZE = 6;
const int MELODY_COUNT = 211;

extern const unsigned char ARPEGGIO_DATA[ARPEGGIO_STEPS];
extern const int FINAL_CHORD[FINAL_CHORD_SIZE];
extern const MelodyNote MELODY[MELODY_COUNT];

/**
 * Frequency of a MIDI note
 *
 * Parameters:
 * int note: MIDI note number (69 is A4, 440 Hz)
 *
 * Returns double: Frequency in Hz
 */
double midiFrequency(int note);

/**
 * Accompaniment note at a sixteenth step
 *
 * Parameters:
 * int step: Sixteenth-note index, wraps modulo SCORE_STEPS
 *
 * Returns int: MIDI note, or -1 once the final chord has started
 */
int arpeggioNote(int step);

/**
 * Melody note sounding at a sixteenth step
 *
 * Parameters:
 * int step: Sixteenth-note index, wraps modulo SCORE_STEPS
 *
 * Returns int: MIDI note, or -1 during a rest
 */
int melodyNote(int step);

#endif
