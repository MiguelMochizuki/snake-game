/**
 * terminal_menu_view.cpp
 * Author: Miguel Mochizuki Silva
 * Description: ASCII start menu implementation
 */
#include "terminal_menu_view.hpp"
#include "terminal_painter.hpp"
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace terminal;

namespace {

const std::size_t INNER = 20;

using Segment = std::pair<const char*, std::string>;

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
	p.put(nullptr, std::string(used < INNER ? INNER - used : 0, ' '));
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
	return {{nullptr, "   "}, {selected ? LABEL : nullptr, (selected ? "> " : "  ") + label}};
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
	return {{nullptr, "   "}, {LABEL, label}, {nullptr, " "}, {NUMBER, number}};
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
	const bool onLevel = snapshot.selected == MenuItem::Level;

	buf << "\033[2J\033[H";
	p.put(WALL, rule);
	p.endl();
	frameRow(p, {});
	const std::string box(14, '#');
	frameRow(p, {{nullptr, "   "}, {BODY, box}});
	frameRow(p, {{nullptr, "   "}, {BODY, "#"}, {HEAD, " SNAKE GAME "}, {BODY, "#"}});
	frameRow(p, {{nullptr, "   "}, {BODY, box}});
	frameRow(p, {});
	frameRow(p, {{nullptr, "   "}, {BODY, "OOOOOO"}, {HEAD, "O"}, {nullptr, "   "}, {FOOD, "*"}});
	frameRow(p, {});

	frameRow(p, itemRow(snapshot.selected == MenuItem::Start, "Start"));
	std::vector<Segment> level = itemRow(onLevel, "Level");
	level.push_back({nullptr, " "});
	level.push_back({onLevel ? LABEL : nullptr, "<"});
	level.push_back({NUMBER, levelName(snapshot.level)});
	level.push_back({onLevel ? LABEL : nullptr, ">"});
	frameRow(p, level);
	frameRow(p, itemRow(snapshot.selected == MenuItem::Quit, "Quit"));

	frameRow(p, {});
	frameRow(p, statRow("Best: ", padded(snapshot.stats.best, 5)));
	frameRow(p, statRow("Games:", padded(snapshot.stats.games, 5)));
	frameRow(p, statRow("Avg:  ", padded(snapshot.stats.average, 5, 1)));
	frameRow(p, {});
	p.put(WALL, rule);
	p.endl();
	p.put(DIM, "W/S pick, A/D level");
	p.endl();
	p.put(DIM, "Enter go, Q quit");
	p.endl();

	out_ << buf.str();
}
