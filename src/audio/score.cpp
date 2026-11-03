/**
 * score.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Lookups into the soundtrack data
 */
#include "snake_game/audio/score.hpp"
#include <cmath>

/**
 * Frequency of a MIDI note
 *
 * Parameters:
 * int note: MIDI note number
 *
 * Returns double: Frequency in Hz
 */
double midiFrequency(int note) {
	return 440.0 * std::pow(2.0, (note - 69) / 12.0);
}

/**
 * Accompaniment note at a sixteenth step
 *
 * Parameters:
 * int step: Sixteenth-note index, wraps modulo SCORE_STEPS
 *
 * Returns int: MIDI note, or -1 once the final chord has started
 */
int arpeggioNote(int step) {
	const int s = ((step % SCORE_STEPS) + SCORE_STEPS) % SCORE_STEPS;
	return s < ARPEGGIO_STEPS ? ARPEGGIO_DATA[s] : -1;
}

/**
 * Melody note sounding at a sixteenth step
 *
 * Parameters:
 * int step: Sixteenth-note index, wraps modulo SCORE_STEPS
 *
 * Returns int: MIDI note, or -1 during a rest
 */
int melodyNote(int step) {
	const int s = ((step % SCORE_STEPS) + SCORE_STEPS) % SCORE_STEPS;
	for (int i = 0; i < MELODY_COUNT; ++i) {
		if (s >= MELODY[i].start && s < MELODY[i].start + MELODY[i].length) {
			return MELODY[i].pitch;
		}
	}
	return -1;
}
