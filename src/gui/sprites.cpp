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
 * How far a shape is pulled in from each side, in pixels; the light comes from the top left, so the shadow
 * layer is drawn whole and the lighter layers are pulled in from the bottom and the right
 *
 * Attributes:
 * int l, int t, int r, int b: Left, top, right and bottom
 */
struct Inset {
	int l, t, r, b;
};

/**
 * Blends two colours
 *
 * Parameters:
 * Color a: The colour at 0 percent
 * Color b: The colour at 100 percent
 * int percent: How much of b, from 0 to 100
 *
 * Returns Color: The blend
 */
Color mix(Color a, Color b, int percent) {
	auto channel = [percent](unsigned char x, unsigned char y) {
		return static_cast<unsigned char>((x * (100 - percent) + y * percent) / 100);
	};
	return Color{channel(a.r, b.r), channel(a.g, b.g), channel(a.b, b.b)};
}

/**
 * The three tones that make a shape look round: shadow, base and light
 *
 * Attributes:
 * Color shadow, Color base, Color light: The tones
 */
struct Ramp {
	Color shadow, base, light;
};

/**
 * Builds the tones around a base colour
 *
 * Parameters:
 * Color base: The base colour
 *
 * Returns Ramp: The shadow, the base itself and the light
 */
Ramp rampOf(Color base) {
	return Ramp{mix(base, Color{4, 10, 8}, 40), base, mix(base, Color{255, 255, 255}, 24)};
}

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

	/**
	 * Fills a circle whose centre is a point of the frame
	 *
	 * Parameters:
	 * IDrawSurface& d: Where to draw
	 * int u, int v: The centre in the frame
	 * int radius: Radius in pixels
	 * Color c: Fill colour
	 */
	void circle(IDrawSurface& d, int u, int v, int radius, Color c) const {
		const Box b = box(u, v, u + 1, v + 1);
		d.fillCircle(b.x0, b.y0, radius, c);
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
 * Fills a diamond
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * int cx, int cy: Centre
 * int half: Half of its width and of its height
 * Color c: Fill colour
 */
void diamond(IDrawSurface& d, int cx, int cy, int half, Color c) {
	for (int dy = -half; dy <= half; ++dy) {
		const int reach = half - std::abs(dy);
		d.fillRect(cx - reach, cy + dy, 2 * reach + 1, 1, c);
	}
}

/**
 * Length of the tongue, which takes the front of the head's square
 *
 * Parameters:
 * int s: Side of the square drawn in a cell
 *
 * Returns int: The length in pixels
 */
int tongueLength(int s) {
	return std::max(2, s / 4);
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
 * Fills the body of one segment, or the head, pulled in by an inset, together with the bridges to its neighbours
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * const std::vector<SpriteCell>& cells: The snake, head first
 * size_t i: Index of the segment
 * int cellPx: Cell side in pixels
 * int s: Side of the square drawn in a cell
 * Facing facing: Direction of travel of the head
 * Inset in: How far to pull the shape in from each side
 * Color c: Fill colour
 */
void fillSegment(IDrawSurface& d, const std::vector<SpriteCell>& cells, size_t i, int cellPx, int s, Facing facing, Inset in, Color c) {
	const SpriteCell cell = cells[i];
	Box box{cell.px, cell.py, cell.px + s, cell.py + s};
	int radius = std::max(1, s / 4);
	if (i == 0) {
		// The head is a blunter, shorter block: the front of its square belongs to the tongue
		box = Frame{cell.px, cell.py, s, facing}.box(0, 0, s - tongueLength(s), s);
		radius = std::max(1, s / 3);
	}
	roundedRect(d, box.x0 + in.l, box.y0 + in.t, box.x1 - box.x0 - in.l - in.r, box.y1 - box.y0 - in.t - in.b, radius, c);

	const int gap = cellPx - s;
	const int across = s - in.l - in.r;
	const int down = s - in.t - in.b;
	for (size_t j : {i - 1, i + 1}) {
		if (j >= cells.size()) {
			continue;  // also catches i - 1 wrapping around for the head
		}
		const int dx = cells[j].px - cell.px;
		const int dy = cells[j].py - cell.py;
		if (dx == cellPx && dy == 0) {
			d.fillRect(cell.px + s / 2, cell.py + in.t, cellPx - s / 2, down, c);
		} else if (dx == -cellPx && dy == 0) {
			d.fillRect(cell.px - gap, cell.py + in.t, gap + s / 2, down, c);
		} else if (dx == 0 && dy == cellPx) {
			d.fillRect(cell.px + in.l, cell.py + s / 2, across, cellPx - s / 2, c);
		} else if (dx == 0 && dy == -cellPx) {
			d.fillRect(cell.px + in.l, cell.py - gap, across, gap + s / 2, c);
		}
	}
}

/**
 * Draws a body segment's markings: a diamond down the back, as on a diamondback
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * SpriteCell cell: The segment's cell
 * int s: Side of the square drawn in a cell
 * const SpritePalette& p: Colours
 */
void drawBodyMarkings(IDrawSurface& d, SpriteCell cell, int s, const SpritePalette& p) {
	const int half = std::max(2, s * 26 / 100);
	const int cx = cell.px + s / 2 - 1;
	const int cy = cell.py + s / 2 - 1;
	diamond(d, cx, cy, half, p.scale);
	if (half >= 4) {
		diamond(d, cx, cy, half - std::max(2, half / 3), mix(p.scale, mix(p.body, Color{255, 255, 255}, 24), 55));
	}
}

/**
 * Draws the head's features: a dark chevron behind the eyes, nostrils, two golden eyes with a slit pupil and a forked tongue
 *
 * Parameters:
 * IDrawSurface& d: Where to draw
 * SpriteCell cell: The head's cell
 * int s: Side of the square drawn in the cell
 * Facing facing: Direction of travel
 * const SpritePalette& p: Colours
 */
void drawHeadFeatures(IDrawSurface& d, SpriteCell cell, int s, Facing facing, const SpritePalette& p) {
	const Frame f{cell.px, cell.py, s, facing};
	const int tongue = tongueLength(s);
	const int mid = s / 2;
	const bool fine = s >= 16;  // below this the details would crowd each other and spill over the cell

	if (fine) {
		// A chevron pointing at the snout, behind the eyes and clear of the centre of the head
		const int w = std::max(1, s / 14);
		const int steps = std::max(1, s * 24 / 100);
		const int tipU = s * 38 / 100;
		const int backU = s * 14 / 100;
		for (int k = 0; k <= steps; ++k) {
			const int u = backU + (tipU - backU) * k / steps;
			const int offset = (s * 22 / 100) * (steps - k) / steps;
			f.fill(d, u, mid - offset - w / 2, u + w, mid - offset - w / 2 + w, p.scale);
			f.fill(d, u, mid + offset - w / 2, u + w, mid + offset - w / 2 + w, p.scale);
		}
		// Nostrils at the front of the snout
		const Color nostril = mix(p.head, Color{0, 0, 0}, 60);
		for (int v : {mid - s / 7, mid + s / 7}) {
			f.fill(d, s - tongue - 3, v, s - tongue - 1, v + 1 + w / 2, nostril);
		}
	}

	// Two golden eyes ahead of the centre and towards the sides, each with a slit pupil across it
	const int eyeU = s * 62 / 100;
	const int radius = fine ? std::max(3, s / 9) : std::max(1, s / 8);
	const int slit = std::max(1, radius / 2);
	for (int v : {s / 4, s - 1 - s / 4}) {
		if (fine) {
			f.circle(d, eyeU, v, radius + 1, mix(p.head, Color{0, 0, 0}, 55));
		}
		f.circle(d, eyeU, v, radius, p.eye);
		f.fill(d, eyeU - slit / 2, v - radius + 1, eyeU - slit / 2 + slit, v + radius, p.pupil);
		if (fine) {
			const Box spot = f.box(eyeU - radius + 1, v - radius + 1, eyeU - radius + 2, v - radius + 2);
			d.fillRect(spot.x0, spot.y0, 1, 1, mix(p.eye, Color{255, 255, 255}, 65));
		}
	}

	// A T-shaped tongue: stem, crossbar and two tines, reaching to the front of the square
	const int w = std::max(1, s / 14);
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
	const Facing facing = facingOf(cells);
	const Ramp body = rampOf(palette.body);
	const Ramp head = rampOf(palette.head);
	// The light comes from the top left: a shadow rim shows on the bottom and the right, a light rim on the top and the left
	const int rim = std::max(1, s / 8);
	const int lightRim = std::max(1, rim / 2);
	const Inset layers[3] = {Inset{0, 0, 0, 0}, Inset{0, 0, rim, rim}, Inset{lightRim, lightRim, rim, rim}};
	// One layer at a time over the whole snake, from the tail to the head, so rims run on unbroken across the joints
	for (int layer = 0; layer < 3; ++layer) {
		for (size_t k = cells.size(); k-- > 0;) {
			const Ramp& ramp = k == 0 ? head : body;
			const Color c = layer == 0 ? ramp.shadow : (layer == 1 ? ramp.light : ramp.base);
			fillSegment(surface, cells, k, cellPx, s, facing, layers[layer], c);
		}
	}
	for (size_t k = cells.size(); k-- > 1;) {
		drawBodyMarkings(surface, cells[k], s, palette);
	}
	if (!cells.empty()) {
		drawHeadFeatures(surface, cells[0], s, facing, palette);
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
	// A shadow crescent on the lower right, the apple itself pulled up and to the left, and a glint
	const int shift = std::max(1, radius / 8);
	surface.fillCircle(cx, cy, radius, mix(palette.apple, Color{4, 10, 8}, 35));
	surface.fillCircle(cx - shift, cy - shift, radius - shift, palette.apple);
	if (radius >= 6) {
		surface.fillCircle(cx - radius / 3 - shift, cy - radius / 3 - shift, std::max(1, radius / 5), mix(palette.apple, Color{255, 255, 255}, 45));
	}
	// A dip at the top where the stem goes in
	const int top = cy - radius;
	surface.fillCircle(cx, top, std::max(1, s / 12), palette.notch);
	const int w = std::max(1, s / 14);
	surface.fillRect(cx - w / 2, std::max(cell.py, top - s / 6), w, top - std::max(cell.py, top - s / 6) + std::max(1, s / 12), palette.stem);
	const int leaf = std::max(1, s / 10);
	surface.fillCircle(cx + s / 6 + leaf / 2, std::max(cell.py + leaf, cell.py + s * 15 / 100), leaf, palette.leaf);
}
