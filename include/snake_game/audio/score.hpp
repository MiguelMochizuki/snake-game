/**
 * score.hpp
 * Author: Miguel Mochizuki Silva
 * Description: The soundtrack as data: the pieces the game plays, as lists of notes in milliseconds
 */
#ifndef SCORE_HPP
#define SCORE_HPP

/**
 * One note of a piece
 *
 * Attributes:
 * int start: When the note begins, in milliseconds from the start of the piece
 * int length: How long it lasts, in milliseconds
 * unsigned char pitch: MIDI note number
 * unsigned char melody: 1 if the note belongs to the melody, 0 for the accompaniment
 * unsigned char velocity: How loud, 1 to 127, 100 being the loudest the piece gets without a crescendo
 */
struct ScoreNote {
	int start;
	int length;
	unsigned char pitch;
	unsigned char melody;
	unsigned char velocity;
};

/**
 * One piece of music
 *
 * Attributes:
 * const char* title: Name of the piece
 * const char* composer: Who wrote it
 * const ScoreNote* notes: The notes, sorted by start
 * int count: Number of notes
 * int endMs: When the piece ends, in milliseconds
 * bool rare: An easter egg, chosen with 1% of the probability
 */
struct Piece {
	const char* title;
	const char* composer;
	const ScoreNote* notes;
	int count;
	int endMs;
	bool rare;
};

// Silence after each piece, before the next one starts
const int REST_BETWEEN_PIECES_MS = 2000;

extern const Piece PIECES[];
extern const int PIECE_COUNT;

/**
 * Frequency of a MIDI note
 *
 * Parameters:
 * int note: MIDI note number (69 is A4, 440 Hz)
 *
 * Returns double: Frequency in Hz
 */
double midiFrequency(int note);

#endif
