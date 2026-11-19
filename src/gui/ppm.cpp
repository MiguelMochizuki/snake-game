/**
 * ppm.cpp
 * Author: Miguel Mochizuki Silva
 * Description: PPM writer implementation
 */
#include "snake_game/gui/ppm.hpp"
#include <cstdio>
#include <vector>

bool writePpm(const Canvas& canvas, const std::string& path) {
	const std::vector<unsigned char> px = canvas.pixelsRgb();
	if (px.empty()) {
		return false;
	}
	std::FILE* f = std::fopen(path.c_str(), "wb");
	if (!f) {
		return false;
	}
	std::fprintf(f, "P6\n%d %d\n255\n", canvas.width(), canvas.height());
	const bool ok = std::fwrite(px.data(), 1, px.size(), f) == px.size();
	return std::fclose(f) == 0 && ok;
}
