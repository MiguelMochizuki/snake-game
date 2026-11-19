/**
 * ppm.cpp
 * Author: Miguel Mochizuki Silva
 * Description: PPM writer implementation
 */
#include "snake_game/gui/ppm.hpp"
#include <cstdio>

bool writePpm(int width, int height, const std::vector<unsigned char>& rgb, const std::string& path) {
	if (width <= 0 || height <= 0 || rgb.size() != static_cast<size_t>(width) * height * 3) {
		return false;
	}
	std::FILE* f = std::fopen(path.c_str(), "wb");
	if (!f) {
		return false;
	}
	std::fprintf(f, "P6\n%d %d\n255\n", width, height);
	const bool ok = std::fwrite(rgb.data(), 1, rgb.size(), f) == rgb.size();
	return std::fclose(f) == 0 && ok;
}
