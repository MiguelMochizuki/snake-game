/**
 * format.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Text formatting helpers shared by the terminal views
 */
#ifndef TERMINAL_FORMAT_HPP
#define TERMINAL_FORMAT_HPP

#include <iomanip>
#include <sstream>
#include <string>

namespace terminal {

/**
 * Formats a number right-aligned in a field
 *
 * Parameters:
 * T value: The number
 * int width: Field width
 * int precision: Digits after the point for floating values (-1 for integers)
 *
 * Returns std::string: The padded text
 */
template <typename T>
std::string padded(T value, int width, int precision = -1) {
	std::ostringstream s;
	s << std::setw(width);
	if (precision >= 0) {
		s << std::fixed << std::setprecision(precision);
	}
	s << value;
	return s.str();
}

}  // namespace terminal

#endif
