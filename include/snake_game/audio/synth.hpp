/**
 * synth.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Soundtrack player and blip synth; produces samples, does no I/O
 */
#ifndef SYNTH_HPP
#define SYNTH_HPP

#include "snake_game/audio/score.hpp"
#include <atomic>

/**
 * Short sound effects mixed on top of the music
 */
enum class Blip { Eat, GameOver, MenuMove, MenuSelect, Win };

/**
 * Plays the soundtrack (the pieces of score.hpp, one after the other, in a loop) and generates the blips.
 * The accompaniment is a soft triangle wave and the melody a square wave.
 *
 * startMusic, stopMusic and trigger may be called from any thread; render must be called from one thread at a time
 *
 * Attributes:
 * int rate_: Sample rate in Hz
 * std::atomic<int> command_: The last music command posted (0 none, 1 start, 2 stop); the render thread takes it
 * once per call, so when several arrive between two calls the last one wins
 * std::atomic<int> blipRequest_: 0 for none, else 1 + the Blip to start
 * bool playing_: Render-thread state: whether the music sounds
 * int piece_, long long pos_, int next_: Render-thread state of the music: the piece playing, the sample position
 * inside it and the next note that has not started yet. Stopping keeps them, so the music goes on where it stopped
 * Voice voices_[MAX_VOICES]: The notes sounding now
 * int blipLeft_, blipTotal_, blipPos_: Render-thread state of the current blip
 * double blipHz_, float blipGain_: Frequency and loudness of the current blip
 */
class Synth {
	/**
	 * A note that is sounding
	 *
	 * Attributes:
	 * double hz: Frequency
	 * int age: Samples played so far
	 * int length: Samples it lasts; the voice is free once age reaches it
	 * bool melody: Whether it is a melody note
	 */
	struct Voice {
		double hz = 0.0;
		int age = 0;
		int length = 0;
		bool melody = false;
	};

	static constexpr int MAX_VOICES = 16;

	int rate_;
	std::atomic<int> command_{0};
	std::atomic<int> blipRequest_{0};
	bool playing_ = false;
	int piece_ = 0;
	long long pos_ = 0;
	int next_ = 0;
	Voice voices_[MAX_VOICES];
	int blipLeft_ = 0;
	int blipTotal_ = 0;
	int blipPos_ = 0;
	double blipHz_ = 0.0;
	float blipGain_ = 0.0f;

	/**
	 * Starts a note, taking the place of the oldest voice when all are busy
	 *
	 * Parameters:
	 * const ScoreNote& note: The note to start
	 */
	void startNote(const ScoreNote& note);

public:
	/**
	 * Constructor for the Synth class
	 *
	 * Parameters:
	 * int sampleRate: Sample rate in Hz (default: 44100)
	 */
	explicit Synth(int sampleRate = 44100);

	/**
	 * Starts the music, where it stopped (from the first piece when it never played)
	 */
	void startMusic();

	/**
	 * Stops the music
	 */
	void stopMusic();

	/**
	 * Starts a blip, replacing one still playing
	 *
	 * Parameters:
	 * Blip blip: Which sound to play
	 */
	void trigger(Blip blip);

	/**
	 * Fills a buffer with mono samples in [-1, 1]
	 *
	 * Parameters:
	 * float* out: Buffer to fill
	 * int frames: Number of samples
	 */
	void render(float* out, int frames);
};

#endif
