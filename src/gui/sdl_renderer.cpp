/**
 * sdl_renderer.cpp
 * Author: Miguel Mochizuki Silva
 * Description: SdlRenderer implementation
 */
#include "snake_game/gui/sdl_renderer.hpp"
#include "snake_game/gui/bitmap_font.hpp"
#include "snake_game/gui/theme.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace {

/**
 * Text scale for a canvas height
 *
 * Parameters:
 * int canvasH: Canvas height in pixels
 *
 * Returns int: 2 on tall canvases, else 1
 */
int textScaleFor(int canvasH) {
	return canvasH >= 480 ? 2 : 1;
}

/**
 * Spacing unit: twice the text scale
 *
 * Parameters:
 * int canvasH: Canvas height in pixels
 *
 * Returns int: Spacing in pixels
 */
int unitFor(int canvasH) {
	return 2 * textScaleFor(canvasH);
}

/**
 * Height of one HUD line, a text row with breathing room
 *
 * Parameters:
 * int canvasH: Canvas height in pixels
 *
 * Returns int: Height in pixels
 */
int lineHeightFor(int canvasH) {
	return font::textHeight(textScaleFor(canvasH)) + 3 * unitFor(canvasH);
}

/**
 * Height reserved around the board: the score line above it and the hint line below it
 *
 * Parameters:
 * int canvasH: Canvas height in pixels
 *
 * Returns int: Height in pixels
 */
int hudHeightFor(int canvasH) {
	return 2 * lineHeightFor(canvasH);
}

/**
 * Side of the square drawn for one cell, leaving a gap between cells when they are big enough
 *
 * Parameters:
 * int cell: Cell side
 *
 * Returns int: Square side
 */
int squareFor(int cell) {
	return cell > 3 ? cell - std::max(1, cell / 16) : cell;
}

/**
 * Formats a number with a fixed number of decimals
 *
 * Parameters:
 * double value: The number
 * int decimals: Digits after the point
 *
 * Returns std::string: The text
 */
std::string fixed(double value, int decimals) {
	std::ostringstream os;
	os << std::fixed << std::setprecision(decimals) << value;
	return os.str();
}

}  // namespace

int cellSizeFor(int canvasW, int canvasH, int cols, int rows, int hudHeight) {
	const int byWidth = canvasW / (cols + 2);
	const int byHeight = (canvasH - hudHeight) / (rows + 2);
	return std::max(1, std::min(byWidth, byHeight));
}

Layout layoutFor(int canvasW, int canvasH, int cols, int rows) {
	const int hud = hudHeightFor(canvasH);
	Layout l;
	l.cell = cellSizeFor(canvasW, canvasH, cols, rows, hud);
	l.originX = std::max(0, (canvasW - l.cell * (cols + 2)) / 2);
	// The score line sits above the board and the hint below it, so the board is centred in what is left between them
	l.originY = lineHeightFor(canvasH) + std::max(0, (canvasH - hud - l.cell * (rows + 2)) / 2);
	return l;
}

SdlRenderer::SdlRenderer(Canvas& canvas, SdlApp& app) : canvas_(canvas), app_(app) {
}

/**
 * Draws the board, snake, food and HUD of a state, without clearing or presenting
 *
 * Parameters:
 * const GameState& state: The state to draw
 */
void SdlRenderer::drawBoard(const GameState& state) {
	const int cols = state.getCols();
	const int rows = state.getRows();
	const Layout l = layoutFor(canvas_.width(), canvas_.height(), cols, rows);
	const int sq = squareFor(l.cell);
	const int u = unitFor(canvas_.height());
	auto cellX = [&](int x) { return l.originX + (x + 1) * l.cell; };
	auto cellY = [&](int y) { return l.originY + (y + 1) * l.cell; };

	// A hairline frame around a slightly lighter playfield; the cell ring around it is breathing room
	const int line = std::max(1, l.cell / 12);
	const int fx = cellX(0) - line - u / 2, fy = cellY(0) - line - u / 2;
	const int fw = cols * l.cell + 2 * line + u, fh = rows * l.cell + 2 * line + u;
	canvas_.fillRect(fx, fy, fw, fh, theme::WALL);
	canvas_.fillRect(fx + line, fy + line, fw - 2 * line, fh - 2 * line, theme::SURFACE);

	const auto& grid = state.getOccupied();
	const int foodX = state.getFood().getX();
	const int foodY = state.getFood().getY();
	const SnakeSegment* head = state.getSnake().getHead();
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			if (!grid[y * cols + x]) {
				continue;
			}
			if (x == foodX && y == foodY) {
				canvas_.fillCircle(cellX(x) + l.cell / 2, cellY(y) + l.cell / 2, std::max(1, l.cell / 2 - std::max(1, l.cell / 8)), theme::FOOD);
			} else if (head && x == head->getX() && y == head->getY()) {
				canvas_.fillRect(cellX(x), cellY(y), sq, sq, theme::HEAD);
			} else {
				canvas_.fillRect(cellX(x), cellY(y), sq, sq, theme::BODY);
			}
		}
	}

	// Score and best on one line above the board, the hint below it, all on the frame's edges
	const int scale = textScaleFor(canvas_.height());
	const int lineH = lineHeightFor(canvas_.height());
	const int textH = font::textHeight(scale);
	const int topY = fy - lineH + (lineH - textH) / 2 - u / 2;
	const std::string score = std::to_string(state.getScore());
	const std::string best = std::to_string(state.getBestScore());
	font::drawText(canvas_, fx, topY, "SCORE", scale, theme::DIM);
	font::drawText(canvas_, fx + font::textWidth("SCORE ", scale), topY, score, scale, theme::NUMBER);
	font::drawText(canvas_, fx + fw - font::textWidth(best, scale), topY, best, scale, theme::NUMBER);
	font::drawText(canvas_, fx + fw - font::textWidth(best + " BEST ", scale), topY, "BEST", scale, theme::DIM);
	font::drawText(canvas_, fx, fy + fh + (lineH - textH) / 2 + u / 2, "WASD or arrows to steer, Q to quit", scale, theme::DIM);
}

/**
 * Clears and redraws the last board and the game over panel if one is showing
 */
void SdlRenderer::redrawAll() {
	canvas_.clear(theme::BACKGROUND);
	if (last_) {
		drawBoard(*last_);
	}
	if (panel_) {
		panel_();
	}
	canvas_.present();
}

void SdlRenderer::reset() {
	last_.reset();
	panel_ = nullptr;
}

void SdlRenderer::clear() {
	canvas_.clear(theme::BACKGROUND);
}

/**
 * Draws the game state and shows it
 *
 * Parameters:
 * const GameState& state: The state to draw
 */
void SdlRenderer::render(const GameState& state) {
	last_ = state;
	panel_ = nullptr;
	drawBoard(state);
	canvas_.present();
	app_.setScene([this] { redrawAll(); });
}

/**
 * Draws the game over panel over the last board and shows it
 *
 * Parameters:
 * Level level: The level that was played
 * int finalScore: The final score
 * const ScoreStats& stats: Best, games played and average of that level
 */
void SdlRenderer::printGameOver(Level level, int finalScore, const ScoreStats& stats) {
	const std::string levelText = levelName(level);
	panel_ = [this, levelText, finalScore, stats] {
		int scale = textScaleFor(canvas_.height());
		const int columns = 26;
		if (font::textWidth(std::string(columns + 4, ' '), scale) > canvas_.width()) {
			scale = 1;
		}
		const int u = 2 * scale;
		const int rowH = font::textHeight(scale) + 3 * u;
		const int pad = 3 * u;
		const int w = std::min(canvas_.width(), font::textWidth(std::string(columns, ' '), scale) + 2 * pad);
		const int rules = 2 * (u + std::max(1, scale / 2) + u);
		const int h = 2 * pad + rowH + 5 * rowH + rowH + rules;
		const int x = std::max(0, (canvas_.width() - w) / 2);
		const int y = std::max(0, (canvas_.height() - h) / 2);
		const int border = std::max(1, scale / 2);
		canvas_.fillRect(x, y, w, h, theme::WALL);
		canvas_.fillRect(x + border, y + border, w - 2 * border, h - 2 * border, theme::SURFACE);
		const int left = x + pad;
		const int right = x + w - pad;
		int ty = y + pad;
		auto textAt = [&](int px, const std::string& text, Color color) {
			font::drawText(canvas_, px, ty + (rowH - font::textHeight(scale)) / 2, text, scale, color);
		};
		auto rule = [&] {
			ty += u;
			canvas_.fillRect(left, ty, right - left, border, theme::WALL);
			ty += border + u;
		};
		textAt(left, "GAME OVER", theme::FOOD);
		ty += rowH;
		rule();
		auto row = [&](const std::string& label, const std::string& value, Color color) {
			textAt(left, label, theme::DIM);
			textAt(right - font::textWidth(value, scale), value, color);
			ty += rowH;
		};
		row("Level", levelText, theme::NUMBER);
		row("Score", std::to_string(finalScore), theme::HEAD);
		row("Best", std::to_string(stats.best), theme::NUMBER);
		row("Games", std::to_string(stats.games), theme::NUMBER);
		row("Average", fixed(stats.average, 1), theme::NUMBER);
		rule();
		int px = left;
		for (const auto& key : {std::make_pair("R", "again"), std::make_pair("M", "menu"), std::make_pair("Q", "quit")}) {
			textAt(px, key.first, theme::HEAD);
			px += font::textWidth(std::string(key.first) + " ", scale);
			textAt(px, key.second, theme::LABEL);
			px += font::textWidth(std::string(key.second) + "  ", scale);
		}
	};
	panel_();
	canvas_.present();
	app_.setScene([this] { redrawAll(); });
}

/**
 * Waits between game steps while handling window events, so the window stays responsive
 *
 * Parameters:
 * int ms: Time to wait in milliseconds
 */
void SdlRenderer::pause(int ms) {
	app_.pump(ms);
}

GameState SdlRenderer::lastState() const {
	return last_ ? *last_ : GameState(20, 20, 0, 0);
}
