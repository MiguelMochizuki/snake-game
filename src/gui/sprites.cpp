/**
 * sprites.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Snake and apple sprite implementation
 */
#include "snake_game/gui/sprites.hpp"
#include <algorithm>
#include <cstdlib>

namespace {

enum class Facing { Right, Left, Down, Up };

/**
 * Pixel rectangle, end exclusive
 *
 * Attributes:
 * int x0, int y0, int x1, int y1: The corners
 */
struct Box {
	int x0, y0, x1, y1;
};

/**
 * Maps the head's own frame (u along the direction of travel, v across it, both from 0 to the square side) onto the surface
 *
 * Attributes:
 * int px, int py: Top-left corner of the cell
 * int s: Side of the square drawn in the cell
 * Facing facing: Direction of travel
 */
struct Frame {
	int px, py, s;
	Facing facing;

	/**
	 * Maps a rectangle of the frame to pixels
	 *
	 * Parameters:
	 * int u0, int v0, int u1, int v1: The rectangle in the frame, end exclusive
	 *
	 * Returns Box: The same rectangle on the surface
	 */
	Box box(int u0, int v0, int u1, int v1) const {
		switch (facing) {
			case Facing::Right: return {px + u0, py + v0, px + u1, py + v1};
			case Facing::Left: return {px + s - u1, py + v0, px + s - u0, py + v1};
			case Facing::Down: return {px + v0, py + u0, px + v1, py + u1};
			case Facing::Up: return {px + v0, py + s - u1, px + v1, py + s - u0};
		}
		return {px, py, px, py};
	}

	/**
	 * Fills a rectangle of the frame
	 *
	 * Parameters:
	 * IDrawSurface& d: Where to draw
	 * int u0, int v0, int u1, int v1: The rectangle in the frame, end exclusive
	 * Color c: Fill colour
	 */
	void fill(IDrawSurface& d, int u0, int v0, int u1, int v1, Color c) const {
		if (u1 <= u0 || v1 <= v0) {
			return;
		}
		const Box b = box(u0, v0, u1, v1);
		d.fillRect(b.x0, b.y0, b.x1 - b.x0, b.y1 - b.y0, c);
	}
};

/**
 * Fills a rectangle with rounded corners
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * int x, int y: Top-left corner
 * int w, int h: Size in pixels
 * int r: Corner radius in pixels, at most half of the smaller side
 * Color c: Fill colour
 */
void roundedRect(IDrawSurface& d, int x, int y, int w, int h, int r, Color c) {
	if (w <= 0 || h <= 0) {
		return;
	}
	r = std::max(0, std::min(r, std::min(w, h) / 2));
	d.fillRect(x + r, y, w - 2 * r, h, c);
	d.fillRect(x, y + r, w, h - 2 * r, c);
	if (r > 0) {
		d.fillCircle(x + r, y + r, r, c);
		d.fillCircle(x + w - 1 - r, y + r, r, c);
		d.fillCircle(x + r, y + h - 1 - r, r, c);
		d.fillCircle(x + w - 1 - r, y + h - 1 - r, r, c);
	}
}

/**
 * Fills the gap between a segment's centre and the next cell, on the side of a neighbour
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * SpriteCell cell: The segment's cell
 * int cellPx: Cell side in pixels
 * int s: Side of the square drawn in the cell
 * int dx, int dy: Direction of the neighbour, each -1, 0 or 1
 * Color c: Fill colour
 */
void halfBridge(IDrawSurface& d, SpriteCell cell, int cellPx, int s, int dx, int dy, Color c) {
	const int gap = cellPx - s;
	if (dx > 0) {
		d.fillRect(cell.px + s / 2, cell.py, cellPx - s / 2, s, c);
	} else if (dx < 0) {
		d.fillRect(cell.px - gap, cell.py, gap + s / 2, s, c);
	} else if (dy > 0) {
		d.fillRect(cell.px, cell.py + s / 2, s, cellPx - s / 2, c);
	} else if (dy < 0) {
		d.fillRect(cell.px, cell.py - gap, s, gap + s / 2, c);
	}
}

/**
 * Joins a segment to the neighbours that are in an adjacent cell
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * const std::vector<SpriteCell>& cells: The snake
 * size_t i: Index of the segment
 * int cellPx: Cell side in pixels
 * int s: Side of the square drawn in a cell
 * Color c: Fill colour
 */
void bridgeNeighbours(IDrawSurface& d, const std::vector<SpriteCell>& cells, size_t i, int cellPx, int s, Color c) {
	for (size_t j : {i - 1, i + 1}) {
		if (j >= cells.size()) {
			continue;  // also catches i - 1 wrapping around for the head
		}
		const int dx = cells[j].px - cells[i].px;
		const int dy = cells[j].py - cells[i].py;
		if ((std::abs(dx) == cellPx && dy == 0) || (dx == 0 && std::abs(dy) == cellPx)) {
			halfBridge(d, cells[i], cellPx, s, (dx > 0) - (dx < 0), (dy > 0) - (dy < 0), c);
		}
	}
}

/**
 * Works out which way the head faces from the first two segments
 *
 * Parameters:
 * const std::vector<SpriteCell>& cells: The snake, head first
 *
 * Returns Facing: The direction of travel; Right for a lone head
 */
Facing facingOf(const std::vector<SpriteCell>& cells) {
	if (cells.size() < 2) {
		return Facing::Right;
	}
	const int dx = cells[0].px - cells[1].px;
	const int dy = cells[0].py - cells[1].py;
	if (std::abs(dx) >= std::abs(dy)) {
		return dx < 0 ? Facing::Left : Facing::Right;
	}
	return dy < 0 ? Facing::Up : Facing::Down;
}

/**
 * Draws the head: a rounded block with two eyes and a forked tongue in front, all inside the cell's square
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * SpriteCell cell: The head's cell
 * int s: Side of the square drawn in the cell
 * Facing facing: Direction of travel
 * const SpritePalette& p: Colours
 */
void drawHead(IDrawSurface& d, SpriteCell cell, int s, Facing facing, const SpritePalette& p) {
	const Frame f{cell.px, cell.py, s, facing};
	const int tongue = std::max(2, s / 4);
	const int radius = std::max(1, s / 4);
	const Box body = f.box(0, 0, s - tongue, s);
	roundedRect(d, body.x0, body.y0, body.x1 - body.x0, body.y1 - body.y0, radius, p.head);

	// Two golden eyes with a slit pupil across them, ahead of the centre and towards the sides
	const int e = std::max(2, s / 5);
	const int slit = std::max(1, e / 3);
	const int u = s * 55 / 100;
	for (int v : {s / 5, s - s / 5 - e}) {
		f.fill(d, u, v, u + e, v + e, p.eye);
		f.fill(d, u + e / 2 - slit / 2, v, u + e / 2 - slit / 2 + slit, v + e, p.pupil);
	}

	// A T-shaped tongue: stem, crossbar and two tines, reaching to the front of the square
	const int w = std::max(1, s / 14);
	const int mid = s / 2;
	const int stemEnd = s - tongue / 2;
	f.fill(d, s - tongue, mid - w / 2, stemEnd, mid - w / 2 + w, p.tongue);
	f.fill(d, stemEnd, mid - 2 * w, std::min(s, stemEnd + w), mid + 2 * w, p.tongue);
	f.fill(d, std::min(s, stemEnd + w), mid - 2 * w, s, mid - w, p.tongue);
	f.fill(d, std::min(s, stemEnd + w), mid + w, s, mid + 2 * w, p.tongue);
}

}  // namespace

int spriteSquare(int cellPx) {
	return cellPx > 3 ? cellPx - std::max(1, cellPx / 16) : cellPx;
}

void drawSnake(IDrawSurface& surface, const std::vector<SpriteCell>& cells, int cellPx, const SpritePalette& palette) {
	const int s = spriteSquare(cellPx);
	if (cellPx < MIN_SPRITE_CELL) {
		for (size_t i = 0; i < cells.size(); ++i) {
			surface.fillRect(cells[i].px, cells[i].py, s, s, i == 0 ? palette.head : palette.body);
		}
		return;
	}
	const int radius = std::max(1, s / 4);
	// From the tail to the head, so the head and its bridge end up on top
	for (size_t k = cells.size(); k-- > 0;) {
		const SpriteCell cell = cells[k];
		if (k == 0) {
			bridgeNeighbours(surface, cells, 0, cellPx, s, palette.head);
			drawHead(surface, cell, s, facingOf(cells), palette);
			continue;
		}
		bridgeNeighbours(surface, cells, k, cellPx, s, palette.body);
		roundedRect(surface, cell.px, cell.py, s, s, radius, palette.body);
		// A scale on every other cell, chosen by the cell so it stays put while the snake moves
		if (((cell.px / cellPx) + (cell.py / cellPx)) % 2 == 0) {
			surface.fillCircle(cell.px + s / 3, cell.py + s / 3, std::max(1, s / 10), palette.scale);
		}
	}
}

void drawApple(IDrawSurface& surface, SpriteCell cell, int cellPx, const SpritePalette& palette) {
	if (cellPx < MIN_SPRITE_CELL) {
		surface.fillCircle(cell.px + cellPx / 2, cell.py + cellPx / 2, std::max(1, cellPx / 2 - std::max(1, cellPx / 8)), palette.apple);
		return;
	}
	const int s = spriteSquare(cellPx);
	const int radius = std::max(2, s * 38 / 100);
	const int cx = cell.px + s / 2;
	const int cy = cell.py + s * 58 / 100;
	surface.fillCircle(cx, cy, radius, palette.apple);
	// A dip at the top where the stem goes in
	const int top = cy - radius;
	surface.fillCircle(cx, top, std::max(1, s / 12), palette.notch);
	const int w = std::max(1, s / 14);
	surface.fillRect(cx - w / 2, std::max(cell.py, top - s / 6), w, top - std::max(cell.py, top - s / 6) + std::max(1, s / 12), palette.stem);
	const int leaf = std::max(1, s / 10);
	surface.fillCircle(cx + s / 6 + leaf / 2, std::max(cell.py + leaf, cell.py + s * 15 / 100), leaf, palette.leaf);
}
