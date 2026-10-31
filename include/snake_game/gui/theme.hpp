/**
 * theme.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Colour palette of the graphical front-end: tinted neutrals, one green accent and one warm colour for the food
 */
#ifndef THEME_HPP
#define THEME_HPP

/**
 * An RGB colour
 *
 * Attributes:
 * unsigned char r, g, b: Red, green and blue, 0 to 255
 */
struct Color {
	unsigned char r, g, b;
};

namespace theme {

// Neutrals share one slightly green tint so nothing looks pasted on
constexpr Color BACKGROUND{15, 18, 17};
constexpr Color SURFACE{24, 30, 28};
constexpr Color WALL{52, 66, 61};
constexpr Color DIM{100, 114, 108};
constexpr Color LABEL{168, 182, 175};
constexpr Color NUMBER{232, 240, 235};

// The accent (snake, selection, key hints) and the single warm colour (food, game over)
constexpr Color BODY{58, 148, 104};
constexpr Color HEAD{124, 214, 162};
constexpr Color FOOD{218, 98, 92};

}  // namespace theme

#endif
