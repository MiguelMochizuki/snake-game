/**
 * ppm.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Saves a canvas as a PPM image, for the demo frames
 */
#ifndef PPM_HPP
#define PPM_HPP

#include "snake_game/gui/canvas.hpp"
#include <string>

/**
 * Writes the canvas as a binary PPM (P6) image
 *
 * Parameters:
 * const Canvas& canvas: The canvas to read back
 * const std::string& path: Output file
 *
 * Returns bool: true if the file was written
 */
bool writePpm(const Canvas& canvas, const std::string& path);

#endif
