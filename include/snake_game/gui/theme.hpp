/**
 * theme.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Colour palette of the graphical front-end, matching the terminal game
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

constexpr Color BACKGROUND{16, 18, 24};
constexpr Color WALL{0, 190, 200};
constexpr Color BODY{40, 170, 70};
constexpr Color HEAD{120, 255, 140};
constexpr Color FOOD{230, 50, 50};
constexpr Color NUMBER{240, 210, 60};
constexpr Color LABEL{225, 225, 225};
constexpr Color DIM{120, 125, 135};

}  // namespace theme

#endif
