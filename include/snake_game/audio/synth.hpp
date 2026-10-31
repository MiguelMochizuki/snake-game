/**
 * synth.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Chord sequencer and blip synth; produces samples, does no I/O
 */
#ifndef SYNTH_HPP
#define SYNTH_HPP

#include <atomic>

/**
 * Short sound effects mixed on top of the music
 */
enum class Blip { Eat, GameOver, MenuMove, MenuSelect };

/**
 * Frequency of one sixteenth-note step of the C - Am - F - G loop
 *
 * Each bar is one chord and plays its notes in the order 1, 2, 3, 2 four times
 *
 * Parameters:
 * int step: Sixteenth-note index, wraps modulo 64
 *
 * Returns double: Frequency in Hz
 */
double stepFrequency(int step);

/**
 * Generates the looping chord music (140 BPM, soft square wave) and the blips
 *
 * startMusic, stopMusic and trigger may be called from any thread; render must be called from one thread at a time
 *
 * Attributes:
 * int rate_: Sample rate in Hz
 * std::atomic<bool> musicOn_: Whether the music should play
 * std::atomic<int> blipRequest_: 0 for none, else 1 + the Blip to start
 * bool wasOn_, int step_, int stepPos_: Render-thread state of the music
 * int blipLeft_, blipTotal_, blipPos_: Render-thread state of the current blip
 * double blipHz_, float blipGain_: Frequency and loudness of the current blip
 */
class Synth {
	int rate_;
	std::atomic<bool> musicOn_{false};
	std::atomic<int> blipRequest_{0};
	bool wasOn_ = false;
	int step_ = 0;
	int stepPos_ = 0;
	int blipLeft_ = 0;
	int blipTotal_ = 0;
	int blipPos_ = 0;
	double blipHz_ = 0.0;
	float blipGain_ = 0.0f;

public:
	/**
	 * Constructor for the Synth class
	 *
	 * Parameters:
	 * int sampleRate: Sample rate in Hz (default: 44100)
	 */
	explicit Synth(int sampleRate = 44100);

	/**
	 * Starts the music from the first chord
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
