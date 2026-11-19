/**
 * ppm.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Saves RGB pixels as a PPM image, for the demo frames and the icon
 */
#ifndef PPM_HPP
#define PPM_HPP

#include <string>
#include <vector>

/**
 * Writes RGB pixels as a binary PPM (P6) image
 *
 * Parameters:
 * int width: Width in pixels
 * int height: Height in pixels
 * const std::vector<unsigned char>& rgb: Row-major RGB bytes, width x height x 3
 * const std::string& path: Output file
 *
 * Returns bool: true if the file was written; false if the pixels do not match the size or the file cannot be written
 */
bool writePpm(int width, int height, const std::vector<unsigned char>& rgb, const std::string& path);

#endif
