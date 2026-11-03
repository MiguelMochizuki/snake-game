/**
 * frame_dumper.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Renderer decorator that writes the drawn frames as PPM files, for recordings
 */
#ifndef FRAME_DUMPER_HPP
#define FRAME_DUMPER_HPP

#include "snake_game/gui/canvas.hpp"
#include "snake_game/render/renderer.hpp"
#include <cstdio>
#include <string>

/**
 * Forwards every call to another renderer and saves every Nth rendered frame and the game over frame as frame-NNNNN.ppm; never waits in real time
 *
 * Attributes:
 * IRenderer& inner_: The renderer that draws
 * Canvas& canvas_: The canvas it draws on
 * std::string dir_: Output directory, must exist
 * int stride_: Save one frame in this many
 * int renders_: Frames rendered so far
 * int written_: Files written so far
 */
class FrameDumper : public IRenderer {
	IRenderer& inner_;
	Canvas& canvas_;
	std::string dir_;
	int stride_;
	int renders_ = 0;
	int written_ = 0;

	/**
	 * Writes the canvas as the next numbered frame
	 */
	void save() {
		char name[32];
		std::snprintf(name, sizeof name, "/frame-%05d.ppm", written_++);
		canvas_.writePpm(dir_ + name);
	}

public:
	/**
	 * Constructor for the FrameDumper class
	 *
	 * Parameters:
	 * IRenderer& inner: The renderer that draws
	 * Canvas& canvas: The canvas it draws on
	 * const std::string& dir: Output directory, must exist
	 * int stride: Save one frame in this many (at least 1)
	 */
	FrameDumper(IRenderer& inner, Canvas& canvas, const std::string& dir, int stride) : inner_(inner), canvas_(canvas), dir_(dir), stride_(stride < 1 ? 1 : stride) {}

	void reset() override { inner_.reset(); }
	void clear() override { inner_.clear(); }

	void render(const GameState& state) override {
		inner_.render(state);
		if (renders_++ % stride_ == 0) {
			save();
		}
	}

	void printGameOver(Level level, int finalScore, const ScoreStats& stats) override {
		inner_.printGameOver(level, finalScore, stats);
		save();
	}

	void pause(int) override {}

	/**
	 * Returns int: Frames rendered so far
	 */
	int renders() const { return renders_; }
};

#endif
