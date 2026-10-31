/**
 * sound.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Sound implementations
 */
#include "snake_game/render/sound.hpp"

/**
 * Constructor for the TerminalBell class
 *
 * Parameters:
 * std::ostream& out: Stream that receives the bell
 */
TerminalBell::TerminalBell(std::ostream& out) : out_(out) {
}

/**
 * Rings the bell for eating food
 */
void TerminalBell::eat() {
	out_ << '\a' << std::flush;
}

/**
 * Rings the bell for the end of a game
 */
void TerminalBell::gameOver() {
	out_ << '\a' << std::flush;
}

/**
 * Does nothing
 */
void NullSound::eat() {
}

/**
 * Does nothing
 */
void NullSound::gameOver() {
}
