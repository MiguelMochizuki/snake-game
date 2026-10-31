/**
 * sdl_menu_view.cpp
 * Author: Miguel Mochizuki Silva
 * Description: SdlMenuView implementation
 */
#include "snake_game/gui/sdl_menu_view.hpp"
#include "snake_game/gui/bitmap_font.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {

const int COLUMNS = 28;  // characters across the widest screen line
const int LINES = 24;    // lines on the tallest screen
const Color HIGHLIGHT{30, 62, 74};

/**
 * Cursor over the lines of one screen
 *
 * Attributes:
 * Canvas& canvas: Where to draw
 * int scale: Pixels per glyph pixel
 * int lineH: Height of one line
 * int x: Left edge of the text
 * int y: Top of the next line
 */
struct Page {
	Canvas& canvas;
	int scale;
	int lineH;
	int x;
	int y;

	/**
	 * Starts a screen: clears, picks the largest text scale that fits and draws the wall frame
	 *
	 * Parameters:
	 * Canvas& c: Where to draw
	 */
	explicit Page(Canvas& c) : canvas(c) {
		canvas.clear(theme::BACKGROUND);
		scale = 1;
		for (int s = 3; s >= 1; --s) {
			if (font::textWidth(std::string(COLUMNS, ' '), s) <= canvas.width() && LINES * (font::textHeight(s) + 4) <= canvas.height()) {
				scale = s;
				break;
			}
		}
		lineH = font::textHeight(scale) + 4;
		x = std::max(0, (canvas.width() - font::textWidth(std::string(COLUMNS, ' '), scale)) / 2);
		y = std::max(0, (canvas.height() - LINES * lineH) / 2);
		canvas.fillRect(x - 4, y - 4, font::textWidth(std::string(COLUMNS, ' '), scale) + 8, 2, theme::WALL);
		canvas.fillRect(x - 4, y + LINES * lineH + 2, font::textWidth(std::string(COLUMNS, ' '), scale) + 8, 2, theme::WALL);
	}

	/**
	 * Draws text pieces side by side on the current line, then moves to the next
	 *
	 * Parameters:
	 * const std::vector<std::pair<std::string, Color>>& pieces: Text and colour of each piece
	 * int indent: Characters of left indent
	 * bool highlighted: Whether to put a highlight bar behind the line
	 */
	void line(const std::vector<std::pair<std::string, Color>>& pieces, int indent = 0, bool highlighted = false) {
		if (highlighted) {
			canvas.fillRect(x, y, font::textWidth(std::string(COLUMNS, ' '), scale), lineH, HIGHLIGHT);
		}
		int px = x + font::textWidth(std::string(indent, ' '), scale);
		for (const auto& piece : pieces) {
			font::drawText(canvas, px, y + 2, piece.first, scale, piece.second);
			px += font::textWidth(piece.first, scale);
		}
		y += lineH;
	}

	/**
	 * Leaves an empty line
	 */
	void blank() {
		y += lineH;
	}
};

using Piece = std::pair<std::string, Color>;

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

/**
 * Draws a menu row: a marker, the label and, for rows with a value, the value between angle brackets
 *
 * Parameters:
 * Page& page: Where to draw
 * bool selected: Whether the row is selected
 * const std::string& label: Row label
 * const std::string* value: Value text, or nullptr for rows without one
 */
void menuRow(Page& page, bool selected, const std::string& label, const std::string* value) {
	const Color text = selected ? theme::LABEL : theme::DIM;
	std::vector<Piece> pieces = {{selected ? "> " : "  ", theme::HEAD}, {label, text}};
	if (value) {
		pieces.push_back({std::string(std::max<int>(1, 14 - static_cast<int>(label.size())), ' '), text});
		pieces.push_back({"<", text});
		pieces.push_back({*value, theme::NUMBER});
		pieces.push_back({">", text});
	}
	page.line(pieces, 3, selected);
}

}  // namespace

SdlMenuView::SdlMenuView(Canvas& canvas, SdlApp& app) : canvas_(canvas), app_(app) {
}

/**
 * Draws the start menu
 *
 * Parameters:
 * const MenuSnapshot& snapshot: Selected row, level, figures and the sound switch
 */
void SdlMenuView::show(const MenuSnapshot& snapshot) {
	auto draw = [this, snapshot] {
		Page page(canvas_);
		page.blank();
		page.line({{"#############", theme::BODY}}, 3);
		page.line({{"# ", theme::BODY}, {"SNAKE GAME", theme::HEAD}, {" #", theme::BODY}}, 3);
		page.line({{"#############", theme::BODY}}, 3);
		page.blank();
		page.line({{"OOOOOO", theme::BODY}, {"O", theme::HEAD}, {"   ", theme::DIM}, {"*", theme::FOOD}}, 3);
		page.blank();
		const std::string level = levelName(snapshot.level);
		const std::string sound = snapshot.sound ? "On" : "Off";
		menuRow(page, snapshot.selected == MenuItem::Start, "Start", nullptr);
		menuRow(page, snapshot.selected == MenuItem::Level, "Level", &level);
		menuRow(page, snapshot.selected == MenuItem::Leaderboard, "Leaderboard", nullptr);
		menuRow(page, snapshot.selected == MenuItem::Help, "How to play", nullptr);
		menuRow(page, snapshot.selected == MenuItem::Sound, "Sound", &sound);
		menuRow(page, snapshot.selected == MenuItem::Quit, "Quit", nullptr);
		page.blank();
		page.line({{"Best:  ", theme::LABEL}, {std::to_string(snapshot.stats.best), theme::NUMBER}}, 3);
		page.line({{"Games: ", theme::LABEL}, {std::to_string(snapshot.stats.games), theme::NUMBER}}, 3);
		page.line({{"Avg:   ", theme::LABEL}, {fixed(snapshot.stats.average, 1), theme::NUMBER}}, 3);
		page.blank();
		page.line({{"W/S pick, A/D change", theme::DIM}}, 3);
		page.line({{"Enter go, Q quit", theme::DIM}}, 3);
		canvas_.present();
	};
	draw();
	app_.setScene(draw);
}

/**
 * Draws the leaderboard of one level
 *
 * Parameters:
 * const LeaderboardSnapshot& snapshot: The level and its top scores
 */
void SdlMenuView::showLeaderboard(const LeaderboardSnapshot& snapshot) {
	auto draw = [this, snapshot] {
		Page page(canvas_);
		page.blank();
		page.line({{"TOP SCORES", theme::HEAD}}, 3);
		page.line({{"<", theme::LABEL}, {levelName(snapshot.level), theme::NUMBER}, {">", theme::LABEL}}, 3);
		page.blank();
		if (snapshot.scores.empty()) {
			page.line({{"No games yet", theme::DIM}}, 3);
		}
		for (size_t i = 0; i < snapshot.scores.size(); ++i) {
			std::string rank = std::to_string(i + 1) + ".";
			if (rank.size() < 3) {
				rank = std::string(3 - rank.size(), ' ') + rank;
			}
			page.line({{rank, theme::LABEL}, {"  ", theme::DIM}, {std::to_string(snapshot.scores[i]), theme::NUMBER}}, 3);
		}
		canvas_.present();
	};
	draw();
	app_.setScene(draw);
}

/**
 * Draws the how-to-play screen
 */
void SdlMenuView::showHelp() {
	auto draw = [this] {
		Page page(canvas_);
		const std::vector<std::string> text = {
			"", "Steer: WASD or", "the arrow keys", "",
			"Eat the red *", "to grow and", "score points.", "",
			"Hitting a wall", "or yourself", "ends the game.", "",
			"After a game:", "R again, M menu", "Q quit",
		};
		page.blank();
		page.line({{"HOW TO PLAY", theme::HEAD}}, 3);
		for (const std::string& line : text) {
			if (line.empty()) {
				page.blank();
			} else {
				page.line({{line, theme::LABEL}}, 3);
			}
		}
		page.blank();
		page.line({{"Enter back", theme::DIM}}, 3);
		canvas_.present();
	};
	draw();
	app_.setScene(draw);
}
