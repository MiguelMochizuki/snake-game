/**
 * sound.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Sound cues of the game and their terminal implementation
 */
#ifndef SOUND_HPP
#define SOUND_HPP

#include <iostream>

/**
 * Sound cues of the game; a terminal bell or a GUI plays them
 */
class ISound {
public:
	/**
	 * Destructor for the ISound interface
	 */
	virtual ~ISound() = default;

	/**
	 * Plays the cue for eating food
	 */
	virtual void eat() = 0;

	/**
	 * Plays the cue for the end of a game
	 */
	virtual void gameOver() = 0;

	/**
	 * Starts the background music; does nothing by default
	 */
	virtual void musicStart() {}

	/**
	 * Stops the background music; does nothing by default
	 */
	virtual void musicStop() {}

	/**
	 * Plays the cue for moving the menu selection; does nothing by default
	 */
	virtual void menuMove() {}

	/**
	 * Plays the cue for choosing a menu item; does nothing by default
	 */
	virtual void menuSelect() {}
};

/**
 * Plays every cue as the terminal bell character
 *
 * Attributes:
 * std::ostream& out_: Stream that receives the bell
 */
class TerminalBell : public ISound {
private:
	std::ostream& out_;

public:
	/**
	 * Constructor for the TerminalBell class
	 *
	 * Parameters:
	 * std::ostream& out: Stream that receives the bell (default: std::cout)
	 */
	explicit TerminalBell(std::ostream& out = std::cout);

	void eat() override;
	void gameOver() override;
};

/**
 * Plays nothing
 */
class NullSound : public ISound {
public:
	void eat() override;
	void gameOver() override;
};

#endif
