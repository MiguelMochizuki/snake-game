/**
 * theme_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that the GUI palette keeps its text readable
 */
#include <gtest/gtest.h>

#include <cmath>

#include "snake_game/gui/theme.hpp"

namespace {

/**
 * Relative luminance of a colour (WCAG 2.x, sRGB)
 *
 * Parameters:
 * Color c: The colour
 *
 * Returns double: Luminance from 0 (black) to 1 (white)
 */
double luminance(Color c) {
	auto channel = [](unsigned char v) {
		const double s = v / 255.0;
		return s <= 0.03928 ? s / 12.92 : std::pow((s + 0.055) / 1.055, 2.4);
	};
	return 0.2126 * channel(c.r) + 0.7152 * channel(c.g) + 0.0722 * channel(c.b);
}

/**
 * Contrast ratio between two colours (WCAG 2.x)
 *
 * Parameters:
 * Color a, Color b: The colours
 *
 * Returns double: Ratio from 1 to 21
 */
double contrast(Color a, Color b) {
	const double la = luminance(a), lb = luminance(b);
	return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

}  // namespace

TEST(Theme, TextColoursAreReadableOnBothSurfaces) {
	for (Color text : {theme::DIM, theme::LABEL, theme::NUMBER, theme::HEAD, theme::FOOD}) {
		EXPECT_GE(contrast(text, theme::BACKGROUND), 4.5) << int(text.r) << "," << int(text.g) << "," << int(text.b) << " on BACKGROUND";
		EXPECT_GE(contrast(text, theme::SURFACE), 4.5) << int(text.r) << "," << int(text.g) << "," << int(text.b) << " on SURFACE";
	}
}

TEST(Theme, HierarchyStaysOrdered) {
	// Dim < label < number, so the dimmer text still reads as secondary after any contrast fix
	EXPECT_LT(contrast(theme::DIM, theme::BACKGROUND), contrast(theme::LABEL, theme::BACKGROUND));
	EXPECT_LT(contrast(theme::LABEL, theme::BACKGROUND), contrast(theme::NUMBER, theme::BACKGROUND));
}
