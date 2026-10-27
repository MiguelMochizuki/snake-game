/**
 * wasd_input_handler_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for the WASD key mapping
 */
#include <gtest/gtest.h>

#include "wasd_input_handler.hpp"

/**
 * Letters map to moves in both cases
 */
TEST(WasdInputHandler, WasdMapsLettersBothCases) {
	WASDInputHandler h;
	const char keys[] = {'w', 'W', 'a', 'A', 's', 'S', 'd', 'D'};
	const Action want[] = {Action::MoveUp, Action::MoveUp, Action::MoveLeft, Action::MoveLeft,
	                       Action::MoveDown, Action::MoveDown, Action::MoveRight, Action::MoveRight};
	for (int i = 0; i < 8; ++i) {
		EXPECT_EQ(h.handle(charKey(keys[i])), want[i]) << keys[i];
	}
}

/**
 * Nothing else maps
 */
TEST(WasdInputHandler, WasdIgnoresEverythingElse) {
	WASDInputHandler h;
	const std::string mapped = "wWaAsSdD";
	for (int c = 0; c < 256; ++c) {
		if (mapped.find(static_cast<char>(c)) == std::string::npos) {
			EXPECT_FALSE(h.handle(charKey(static_cast<char>(c)))) << c;
		}
	}
	for (KeyKind k : {KeyKind::Up, KeyKind::Down, KeyKind::Left, KeyKind::Right, KeyKind::CtrlC, KeyKind::Unknown}) {
		EXPECT_FALSE(h.handle(specialKey(k)));
	}
}
