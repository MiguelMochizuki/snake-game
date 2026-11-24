/**
 * score.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Lookups into the soundtrack data
 */
#include "snake_game/audio/score.hpp"
#include <cmath>

double midiFrequency(int note) {
	return 440.0 * std::pow(2.0, (note - 69) / 12.0);
}
