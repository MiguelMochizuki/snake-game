/**
 * terminal_menu_view.cpp
 * Author: Miguel Mochizuki Silva
 * Description: ASCII start menu implementation
 */
#include "snake_game/menu/terminal_menu_view.hpp"
#include "snake_game/terminal/format.hpp"
#include "snake_game/terminal/painter.hpp"
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace terminal;

namespace {

const std::size_t INNER = 20;
const char* const RESET_NOTICE = "Scores reset: score file was edited or damaged";

using Segment = std::pair<Style, std::string>;

/**
 * Writes one framed row, padding the segments to the inner width
 *
 * Parameters:
 * Painter& p: Where to write
 * const std::vector<Segment>& segments: Styled pieces of the row, left to right
 *
 * Returns void:
 */
void frameRow(Painter& p, const std::vector<Segment>& segments) {
	std::size_t used = 0;
	p.put(WALL, "#");
	for (const Segment& s : segments) {
		p.put(s.first, s.second);
		used += s.second.size();
	}
	p.put(PLAIN, std::string(used < INNER ? INNER - used : 0, ' '));
	p.put(WALL, "#");
	p.endl();
}

/**
 * Builds a menu row: marker, label and an optional level value
 *
 * Parameters:
 * bool selected: Whether the row is selected
 * const std::string& label: Row label
 *
 * Returns std::vector<Segment>: The row's segments
 */
std::vector<Segment> itemRow(bool selected, const std::string& label) {
	return {{PLAIN, "   "}, {selected ? LABEL : PLAIN, (selected ? "> " : "  ") + label}};
}

/**
 * Builds a menu row whose value is shown between angle brackets
 *
 * Parameters:
 * bool selected: Whether the row is selected
 * const std::string& label: Row label
 * const std::string& value: Value shown in yellow
 *
 * Returns std::vector<Segment>: The row's segments
 */
std::vector<Segment> valueRow(bool selected, const std::string& label, const std::string& value) {
	std::vector<Segment> row = itemRow(selected, label);
	row.push_back({PLAIN, " "});
	row.push_back({selected ? LABEL : PLAIN, "<"});
	row.push_back({NUMBER, value});
	row.push_back({selected ? LABEL : PLAIN, ">"});
	return row;
}

/**
 * Builds a stats row: label then a right-aligned yellow number
 *
 * Parameters:
 * const std::string& label: Label, padded by the caller to 6 characters
 * const std::string& number: Already formatted number
 *
 * Returns std::vector<Segment>: The row's segments
 */
std::vector<Segment> statRow(const std::string& label, const std::string& number) {
	return {{PLAIN, "   "}, {LABEL, label}, {PLAIN, " "}, {NUMBER, number}};
}

}  // namespace

/**
 * Constructor for the TerminalMenuView class
 *
 * Parameters:
 * std::ostream& out: Stream that receives all output
 * bool color: Whether to emit ANSI SGR color codes
 */
TerminalMenuView::TerminalMenuView(std::ostream& out, bool color) : out_(out), color_(color) {
}

/**
 * Clears the screen and draws the menu
 *
 * Parameters:
 * const MenuSnapshot& snapshot: What to draw
 *
 * Returns void:
 */
void TerminalMenuView::show(const MenuSnapshot& snapshot) {
	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(INNER + 2, '#');

	buf << "\033[2J\033[H";
	p.put(WALL, rule);
	p.endl();
	frameRow(p, {});
	const std::string box(14, '#');
	frameRow(p, {{PLAIN, "   "}, {BODY, box}});
	frameRow(p, {{PLAIN, "   "}, {BODY, "#"}, {HEAD, " SNAKE GAME "}, {BODY, "#"}});
	frameRow(p, {{PLAIN, "   "}, {BODY, box}});
	frameRow(p, {});
	frameRow(p, {{PLAIN, "   "}, {BODY, "OOOOOO"}, {HEAD, "O"}, {PLAIN, "   "}, {FOOD, "*"}});
	frameRow(p, {});

	frameRow(p, itemRow(snapshot.selected == MenuItem::Start, "Start"));
	frameRow(p, valueRow(snapshot.selected == MenuItem::Level, "Level", levelName(snapshot.level)));
	frameRow(p, itemRow(snapshot.selected == MenuItem::Leaderboard, "Leaderboard"));
	frameRow(p, itemRow(snapshot.selected == MenuItem::Help, "How to play"));
	frameRow(p, valueRow(snapshot.selected == MenuItem::Sound, "Sound", snapshot.sound ? "On" : "Off"));
	frameRow(p, valueRow(snapshot.selected == MenuItem::Practice, "Practice", snapshot.practice ? "On" : "Off"));
	frameRow(p, itemRow(snapshot.selected == MenuItem::Quit, "Quit"));

	frameRow(p, {});
	frameRow(p, statRow("Best: ", padded(snapshot.stats.best, 5)));
	frameRow(p, statRow("Games:", padded(snapshot.stats.games, 5)));
	frameRow(p, statRow("Avg:  ", padded(snapshot.stats.average, 5, 1)));
	frameRow(p, {});
	p.put(WALL, rule);
	p.endl();
	if (snapshot.scoresReset) {
		p.put(FOOD, RESET_NOTICE);
		p.endl();
	}
	if (!snapshot.stats.unsavedDir.empty()) {
		p.put(FOOD, "Scores cannot be saved: " + snapshot.stats.unsavedDir);
		p.endl();
	}
	p.put(DIM, "W/S pick, A/D change");
	p.endl();
	p.put(DIM, "Enter go, Q quit");
	p.endl();

	out_ << buf.str();
}

/**
 * Clears the screen and draws the top scores of a level
 *
 * Parameters:
 * const LeaderboardSnapshot& snapshot: What to draw
 *
 * Returns void:
 */
void TerminalMenuView::showLeaderboard(const LeaderboardSnapshot& snapshot) {
	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(INNER + 2, '#');

	buf << "\033[2J\033[H";
	p.put(WALL, rule);
	p.endl();
	frameRow(p, {});
	frameRow(p, {{PLAIN, "   "}, {HEAD, "TOP SCORES"}});
	frameRow(p, {{PLAIN, "   "}, {LABEL, "<"}, {NUMBER, levelName(snapshot.level)}, {LABEL, ">"}});
	frameRow(p, {});
	if (snapshot.scores.empty()) {
		frameRow(p, {{PLAIN, "   "}, {DIM, "No games yet"}});
	}
	for (std::size_t i = 0; i < snapshot.scores.size(); ++i) {
		frameRow(p, {{PLAIN, "   "}, {LABEL, padded(i + 1, 2) + "."}, {PLAIN, "  "},
			{NUMBER, std::to_string(snapshot.scores[i])}});
	}
	frameRow(p, {});
	p.put(WALL, rule);
	p.endl();
	p.put(DIM, "A/D level, C clear");
	p.endl();
	p.put(DIM, "Enter back");
	p.endl();

	out_ << buf.str();
}

/**
 * Clears the screen and draws the how-to-play page
 *
 * Returns void:
 */
void TerminalMenuView::showHelp() {
	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(INNER + 2, '#');
	const std::vector<std::string> text = {
		"", "Steer: WASD or", "the arrow keys.", "P pauses; Q ends", "the game.", "",
		"Eat the red *", "to grow and", "score points.", "",
		"Hitting a wall", "or yourself", "ends the game.", "",
		"After a game:", "R again, M menu", "Q quit",
	};

	buf << "\033[2J\033[H";
	p.put(WALL, rule);
	p.endl();
	frameRow(p, {});
	frameRow(p, {{PLAIN, "   "}, {HEAD, "HOW TO PLAY"}});
	for (const std::string& line : text) {
		if (line.empty()) {
			frameRow(p, {});
		} else {
			frameRow(p, {{PLAIN, "   "}, {PLAIN, line}});
		}
	}
	frameRow(p, {});
	p.put(WALL, rule);
	p.endl();
	p.put(DIM, "Enter back");
	p.endl();

	out_ << buf.str();
}

/**
 * Draws the question asked before the saved scores of a level are deleted
 *
 * Parameters:
 * Level level: The level that would be cleared
 */
void TerminalMenuView::showConfirmClear(Level level) {
	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(INNER + 2, '#');
	const std::vector<std::string> text = {
		"", "Delete every saved", "score of " + std::string(levelName(level)) + "?", "", "This cannot be", "undone.",
	};

	buf << "\033[2J\033[H";
	p.put(WALL, rule);
	p.endl();
	frameRow(p, {});
	frameRow(p, {{PLAIN, "   "}, {FOOD, "CLEAR SCORES"}});
	for (const std::string& line : text) {
		if (line.empty()) {
			frameRow(p, {});
		} else {
			frameRow(p, {{PLAIN, "   "}, {PLAIN, line}});
		}
	}
	frameRow(p, {});
	p.put(WALL, rule);
	p.endl();
	p.put(DIM, "Enter delete, Q keep");
	p.endl();

	out_ << buf.str();
}
