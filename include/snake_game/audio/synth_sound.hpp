/**
 * synth_sound.hpp
 * Author: Miguel Mochizuki Silva
 * Description: ISound that plays the cues and the music through a Synth
 */
#ifndef SYNTH_SOUND_HPP
#define SYNTH_SOUND_HPP

#include "snake_game/audio/synth.hpp"
#include "snake_game/render/sound.hpp"

/**
 * Turns every sound cue into a Synth call
 *
 * Attributes:
 * Synth& synth_: The synth that makes the sound, must outlive this object
 */
class SynthSound : public ISound {
	Synth& synth_;

public:
	/**
	 * Constructor for the SynthSound class
	 *
	 * Parameters:
	 * Synth& synth: The synth that makes the sound
	 */
	explicit SynthSound(Synth& synth) : synth_(synth) {}

	void eat() override { synth_.trigger(Blip::Eat); }
	void gameOver() override { synth_.trigger(Blip::GameOver); }
	void musicStart() override { synth_.startMusic(); }
	void musicStop() override { synth_.stopMusic(); }
	void menuMove() override { synth_.trigger(Blip::MenuMove); }
	void menuSelect() override { synth_.trigger(Blip::MenuSelect); }
};

#endif
