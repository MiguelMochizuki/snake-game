/**
 * synth.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Soundtrack player and blip synth; produces samples, does no I/O
 */
#ifndef SYNTH_HPP
#define SYNTH_HPP

#include <atomic>

/**
 * Short sound effects mixed on top of the music
 */
enum class Blip { Eat, GameOver, MenuMove, MenuSelect };

/**
 * Plays the soundtrack (Gounod's Ave Maria over Bach's C major Prelude, see score.hpp) in a loop and
 * generates the blips. The accompaniment is a soft triangle wave and the melody a square wave.
 *
 * startMusic, stopMusic and trigger may be called from any thread; render must be called from one thread at a time
 *
 * Attributes:
 * int rate_: Sample rate in Hz
 * std::atomic<bool> musicOn_: Whether the music should play
 * std::atomic<int> blipRequest_: 0 for none, else 1 + the Blip to start
 * bool wasOn_, int step_, int stepPos_, int melodyIdx_: Render-thread state of the music: current sixteenth,
 * sample position inside it, and the next melody note that has not finished yet
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
	int melodyIdx_ = 0;
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
	 * Starts the music from the beginning
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
