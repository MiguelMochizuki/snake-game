/**
 * sdl_menu_view.cpp
 * Author: Miguel Mochizuki Silva
 * Description: SdlMenuView implementation
 */
#include "snake_game/gui/sdl_menu_view.hpp"
#include "snake_game/gui/bitmap_font.hpp"
#include <algorithm>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {

const int COLUMNS = 24;  // characters across the content column
const int TALLEST = 15;  // rows of text the tallest screen needs, used to pick the text scale

/**
 * Text scale for a canvas: the largest that fits the content column and the tallest screen
 *
 * Parameters:
 * int canvasW, int canvasH: Canvas size in pixels
 *
 * Returns int: Scale from 1 to 4
 */
int scaleFor(int canvasW, int canvasH) {
	for (int s = 4; s > 1; --s) {
		if (COLUMNS * font::GLYPH * s + 8 * s <= canvasW && TALLEST * 14 * s <= canvasH) {
			return s;
		}
	}
	return 1;
}

/**
 * One screen laid out as a single column: everything is spaced in multiples of u = 2 x the text scale, and a dry pass measures the height so the block can be centred
 *
 * Attributes:
 * Canvas& canvas: Where to draw
 * bool dry: true to only advance y, drawing nothing
 * int scale: Pixels per glyph pixel
 * int u: Spacing unit
 * int x: Left edge of the column
 * int w: Width of the column
 * int y: Top of the next element
 */
struct Page {
	Canvas& canvas;
	bool dry;
	int scale;
	int u;
	int x;
	int w;
	int y;

	/**
	 * Starts a pass over a screen
	 *
	 * Parameters:
	 * Canvas& c: Where to draw
	 * bool dryRun: Whether to only measure
	 * int top: Where the block starts
	 */
	Page(Canvas& c, bool dryRun, int top) : canvas(c), dry(dryRun) {
		scale = scaleFor(c.width(), c.height());
		u = 2 * scale;
		w = COLUMNS * font::GLYPH * scale;
		x = std::max(0, (c.width() - w) / 2);
		y = top;
		if (!dry) {
			canvas.clear(theme::BACKGROUND);
		}
	}

	/**
	 * Returns int: Height of one text row
	 */
	int rowH() const {
		return font::GLYPH * scale + 3 * u;
	}

	/**
	 * Draws text at a position, vertically centred in a row
	 *
	 * Parameters:
	 * int px: Left edge
	 * int rowTop: Top of the row
	 * const std::string& text: The text
	 * Color color: Text colour
	 * int textScale: Pixels per glyph pixel
	 */
	void text(int px, int rowTop, const std::string& text, Color color, int textScale) {
		if (!dry) {
			font::drawText(canvas, px, rowTop + (rowH() - font::textHeight(textScale)) / 2, text, textScale, color);
		}
	}

	/**
	 * Draws text with its right edge on the column's right edge
	 */
	void textRight(int rowTop, const std::string& text, Color color) {
		this->text(x + w - u - font::textWidth(text, scale), rowTop, text, color, scale);
	}

	/**
	 * Empty space
	 *
	 * Parameters:
	 * int units: Height in spacing units
	 */
	void gap(int units) {
		y += units * u;
	}

	/**
	 * A hairline across the column with spacing above and below
	 */
	void rule() {
		y += u;
		if (!dry) {
			canvas.fillRect(x, y, w, std::max(1, scale / 2), theme::WALL);
		}
		y += std::max(1, scale / 2) + u;
	}

	/**
	 * The screen title in a larger size
	 *
	 * Parameters:
	 * const std::string& title: The title
	 */
	void title(const std::string& title) {
		const int ts = scale + 1;
		if (!dry) {
			font::drawText(canvas, x + 2 * u, y, title, ts, theme::HEAD);
		}
		y += font::textHeight(ts);
	}

	/**
	 * The snake and its food drawn from the same blocks as the board
	 */
	void art() {
		const int cell = 3 * u;
		const int sq = cell - std::max(1, scale / 2);
		if (!dry) {
			int px = x + 2 * u;
			for (int i = 0; i < 6; ++i, px += cell) {
				canvas.fillRect(px, y, sq, sq, theme::BODY);
			}
			canvas.fillRect(px, y, sq, sq, theme::HEAD);
			px += 3 * cell;
			canvas.fillCircle(px + cell / 2, y + cell / 2, std::max(1, cell / 2 - 1), theme::FOOD);
		}
		y += cell;
	}

	/**
	 * A menu row: the selected one gets a surface bar with an accent edge and brighter text; a value is right-aligned and shows arrows while selected
	 *
	 * Parameters:
	 * const std::string& label: Row label
	 * const std::string* value: Value text, or nullptr
	 * bool selected: Whether the row is selected
	 */
	void row(const std::string& label, const std::string* value, bool selected) {
		if (!dry && selected) {
			canvas.fillRect(x, y, w, rowH(), theme::SURFACE);
			canvas.fillRect(x, y, std::max(2, scale), rowH(), theme::HEAD);
		}
		text(x + 2 * u, y, label, selected ? theme::NUMBER : theme::LABEL, scale);
		if (value) {
			const std::string shown = selected ? "< " + *value + " >" : *value;
			if (!dry) {
				const int right = x + w - u;
				const int left = right - font::textWidth(shown, scale);
				if (selected) {
					text(left, y, "<", theme::HEAD, scale);
					text(right - font::textWidth(">", scale), y, ">", theme::HEAD, scale);
					text(right - font::textWidth(" " + *value + " >", scale) + font::textWidth(" ", scale), y, *value, theme::NUMBER, scale);
				} else {
					text(left, y, shown, theme::NUMBER, scale);
				}
			}
		}
		y += rowH();
	}

	/**
	 * A figure: dim label on the left, bright value on the right
	 *
	 * Parameters:
	 * const std::string& label: Label
	 * const std::string& value: Value
	 * Color valueColor: Colour of the value
	 */
	void stat(const std::string& label, const std::string& value, Color valueColor = theme::NUMBER) {
		text(x + 2 * u, y, label, theme::DIM, scale);
		text(x + w - u - font::textWidth(value, scale), y, value, valueColor, scale);
		y += rowH();
	}

	/**
	 * A line of plain text
	 *
	 * Parameters:
	 * const std::string& line: The text
	 * Color color: Text colour
	 */
	void line(const std::string& line, Color color) {
		text(x + 2 * u, y, line, color, scale);
		y += rowH();
	}
};

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
 * Draws a screen twice: once to measure it, once centred vertically
 *
 * Parameters:
 * Canvas& canvas: Where to draw
 * const std::function<void(Page&)>& content: Lays out the screen on a page
 */
void drawScreen(Canvas& canvas, const std::function<void(Page&)>& content) {
	Page measure(canvas, true, 0);
	content(measure);
	const int top = std::max(measure.u, (canvas.height() - measure.y) / 2);
	Page page(canvas, false, top);
	content(page);
	canvas.present();
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
		drawScreen(canvas_, [&](Page& p) {
			const std::string level = levelName(snapshot.level);
			const std::string sound = snapshot.sound ? "On" : "Off";
			p.title("SNAKE");
			p.gap(2);
			p.art();
			p.rule();
			p.row("Start", nullptr, snapshot.selected == MenuItem::Start);
			p.row("Level", &level, snapshot.selected == MenuItem::Level);
			p.row("Leaderboard", nullptr, snapshot.selected == MenuItem::Leaderboard);
			p.row("How to play", nullptr, snapshot.selected == MenuItem::Help);
			p.row("Sound", &sound, snapshot.selected == MenuItem::Sound);
			p.row("Quit", nullptr, snapshot.selected == MenuItem::Quit);
			p.rule();
			p.stat("Best", std::to_string(snapshot.stats.best));
			p.stat("Games", std::to_string(snapshot.stats.games));
			p.stat("Average", fixed(snapshot.stats.average, 1));
			p.rule();
			p.line("W/S pick  A/D change", theme::DIM);
			p.line("Enter go  Q quit", theme::DIM);
		});
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
		drawScreen(canvas_, [&](Page& p) {
			p.title("TOP SCORES");
			p.gap(2);
			const std::string level = levelName(snapshot.level);
			p.row("Level", &level, true);
			p.rule();
			if (snapshot.scores.empty()) {
				p.line("No games yet", theme::DIM);
			}
			for (size_t i = 0; i < snapshot.scores.size(); ++i) {
				p.stat(std::to_string(i + 1) + ".", std::to_string(snapshot.scores[i]), i == 0 ? theme::HEAD : theme::NUMBER);
			}
			p.rule();
			p.line("A/D level  Enter back", theme::DIM);
		});
	};
	draw();
	app_.setScene(draw);
}

/**
 * Draws the how-to-play screen
 */
void SdlMenuView::showHelp() {
	auto draw = [this] {
		drawScreen(canvas_, [&](Page& p) {
			p.title("HOW TO PLAY");
			p.gap(2);
			p.rule();
			const std::vector<std::vector<std::string>> paragraphs = {
				{"Steer: WASD or", "the arrow keys"},
				{"Eat the red dot", "to grow and", "score points."},
				{"Hitting a wall", "or yourself", "ends the game."},
				{"After a game:", "R again, M menu", "Q quit"},
			};
			for (size_t i = 0; i < paragraphs.size(); ++i) {
				if (i > 0) {
					p.gap(2);
				}
				for (const std::string& line : paragraphs[i]) {
					p.line(line, theme::LABEL);
				}
			}
			p.rule();
			p.line("Enter back", theme::DIM);
		});
	};
	draw();
	app_.setScene(draw);
}
