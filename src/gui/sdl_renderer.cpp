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

const int HUD_LINES = 3;

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
 * Height reserved below the board for the HUD text
 *
 * Parameters:
 * int canvasH: Canvas height in pixels
 *
 * Returns int: Height in pixels
 */
int hudHeightFor(int canvasH) {
	return HUD_LINES * (font::textHeight(textScaleFor(canvasH)) + 4) + 8;
}

/**
 * Side of the square drawn for one cell, leaving a 1 px gap when cells are big enough
 *
 * Parameters:
 * int cell: Cell side
 *
 * Returns int: Square side
 */
int squareFor(int cell) {
	return cell > 2 ? cell - 1 : cell;
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
	l.originY = std::max(0, (canvasH - hud - l.cell * (rows + 2)) / 2);
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
	auto cellX = [&](int x) { return l.originX + (x + 1) * l.cell; };
	auto cellY = [&](int y) { return l.originY + (y + 1) * l.cell; };

	for (int x = -1; x <= cols; ++x) {
		canvas_.fillRect(cellX(x), cellY(-1), sq, sq, theme::WALL);
		canvas_.fillRect(cellX(x), cellY(rows), sq, sq, theme::WALL);
	}
	for (int y = 0; y < rows; ++y) {
		canvas_.fillRect(cellX(-1), cellY(y), sq, sq, theme::WALL);
		canvas_.fillRect(cellX(cols), cellY(y), sq, sq, theme::WALL);
	}

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
				canvas_.fillCircle(cellX(x) + l.cell / 2, cellY(y) + l.cell / 2, std::max(1, l.cell / 2 - 1), theme::FOOD);
			} else if (head && x == head->getX() && y == head->getY()) {
				canvas_.fillRect(cellX(x), cellY(y), sq, sq, theme::HEAD);
			} else {
				canvas_.fillRect(cellX(x), cellY(y), sq, sq, theme::BODY);
			}
		}
	}

	const int scale = textScaleFor(canvas_.height());
	const int lineH = font::textHeight(scale) + 4;
	const int textX = l.originX;
	int textY = l.originY + (rows + 2) * l.cell + 4;
	font::drawText(canvas_, textX, textY, "Score:", scale, theme::LABEL);
	font::drawText(canvas_, textX + font::textWidth("Score: ", scale), textY, std::to_string(state.getScore()), scale, theme::NUMBER);
	textY += lineH;
	font::drawText(canvas_, textX, textY, "Best:", scale, theme::LABEL);
	font::drawText(canvas_, textX + font::textWidth("Best: ", scale), textY, std::to_string(state.getBestScore()), scale, theme::NUMBER);
	textY += lineH;
	font::drawText(canvas_, textX, textY, "Controls: WASD to move, Q to quit", scale, theme::DIM);
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
		if (font::textWidth(std::string(30, ' '), scale) > canvas_.width()) {
			scale = 1;
		}
		const int lineH = font::textHeight(scale) + 4;
		const int lines = 9;
		const int w = font::textWidth(std::string(30, ' '), scale);
		const int h = lines * lineH + 8;
		const int x = std::max(0, (canvas_.width() - w) / 2);
		const int y = std::max(0, (canvas_.height() - h) / 2);
		canvas_.fillRect(x, y, w, h, theme::BACKGROUND);
		canvas_.fillRect(x, y, w, 2, theme::WALL);
		canvas_.fillRect(x, y + h - 2, w, 2, theme::WALL);
		int ty = y + 6;
		const int labelX = x + 2 * font::GLYPH * scale;
		font::drawText(canvas_, x + 9 * font::GLYPH * scale, ty, "GAME OVER!", scale, theme::FOOD);
		ty += 2 * lineH;
		auto row = [&](const std::string& label, const std::string& value) {
			font::drawText(canvas_, labelX, ty, label, scale, theme::LABEL);
			font::drawText(canvas_, x + w - 2 * font::GLYPH * scale - font::textWidth(value, scale), ty, value, scale, theme::NUMBER);
			ty += lineH;
		};
		row("Level:", levelText);
		row("Final Score:", std::to_string(finalScore));
		row("Best Score:", std::to_string(stats.best));
		row("Games Played:", std::to_string(stats.games));
		row("Average Score:", fixed(stats.average, 1));
		ty += lineH / 2;
		font::drawText(canvas_, labelX, ty, "[R]", scale, theme::HEAD);
		font::drawText(canvas_, labelX + font::textWidth("[R] ", scale), ty, "Restart", scale, theme::LABEL);
		font::drawText(canvas_, labelX + font::textWidth("[R] Restart  ", scale), ty, "[M]", scale, theme::WALL);
		font::drawText(canvas_, labelX + font::textWidth("[R] Restart  [M] ", scale), ty, "Menu", scale, theme::LABEL);
		ty += lineH;
		font::drawText(canvas_, labelX, ty, "[Q]", scale, theme::FOOD);
		font::drawText(canvas_, labelX + font::textWidth("[Q] ", scale), ty, "Quit", scale, theme::LABEL);
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
