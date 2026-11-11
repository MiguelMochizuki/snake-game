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
enum class Blip { Eat, GameOver, MenuMove, MenuSelect, Win };

/**
 * Plays the soundtrack (Gounod's Ave Maria over Bach's C major Prelude, see score.hpp) in a loop and
 * generates the blips. The accompaniment is a soft triangle wave and the melody a square wave.
 *
 * startMusic, stopMusic, finale and trigger may be called from any thread; render must be called from one thread at a time
 *
 * Attributes:
 * int rate_: Sample rate in Hz
 * std::atomic<int> command_: The last music command posted (0 none, 1 start, 2 stop, 3 finale); the render thread takes it
 * once per call, so when several arrive between two calls the last one wins
 * std::atomic<int> blipRequest_: 0 for none, else 1 + the Blip to start
 * bool playing_, bool finaleActive_: Render-thread state: whether the music sounds, and whether it is the ending, after which it stops by itself
 * int step_, int stepPos_, int melodyIdx_: Render-thread state of the music: current sixteenth,
 * sample position inside it, and the next melody note that has not finished yet
 * int blipLeft_, blipTotal_, blipPos_: Render-thread state of the current blip
 * double blipHz_, float blipGain_: Frequency and loudness of the current blip
 */
class Synth {
	int rate_;
	std::atomic<int> command_{0};
	std::atomic<int> blipRequest_{0};
	bool playing_ = false;
	bool finaleActive_ = false;
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
	 * Cuts the loop and plays only the last bars of the piece, once; the music stops by itself at the end.
	 * Starting the music again, even during the ending, starts it over from the beginning
	 */
	void finale();

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
