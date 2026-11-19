/**
 * sprites_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the snake and apple sprites, drawn on a PixelSurface so they need no SDL
 */
#include <gtest/gtest.h>

#include <vector>

#include "snake_game/gui/pixel_surface.hpp"
#include "snake_game/gui/sprites.hpp"
#include "snake_game/gui/theme.hpp"

namespace {

const int CELL = 40;
const Color BG = theme::SURFACE;

// Every role gets its own colour, so a pixel says which part of the sprite drew it
const SpritePalette PALETTE{
	Color{10, 100, 10},   // body
	Color{20, 200, 20},   // head
	Color{5, 60, 5},      // scale
	Color{250, 250, 0},   // eye
	Color{1, 1, 1},       // pupil
	Color{240, 20, 150},  // tongue
	Color{220, 30, 30},   // apple
	Color{120, 80, 40},   // stem
	Color{140, 220, 60},  // leaf
	BG,                   // notch
};

/**
 * Reads one pixel
 *
 * Parameters:
 * const PixelSurface& s: The surface
 * int x, int y: Pixel position
 *
 * Returns Color: The pixel colour
 */
Color at(const PixelSurface& s, int x, int y) {
	const size_t i = (static_cast<size_t>(y) * s.width() + x) * 3;
	return Color{s.pixelsRgb()[i], s.pixelsRgb()[i + 1], s.pixelsRgb()[i + 2]};
}

/**
 * Compares two colours
 *
 * Parameters:
 * Color a, Color b: The colours
 *
 * Returns bool: true if all three channels match
 */
bool same(Color a, Color b) {
	return a.r == b.r && a.g == b.g && a.b == b.b;
}

/**
 * Counts the pixels of a colour inside a rectangle
 *
 * Parameters:
 * const PixelSurface& s: The surface
 * Color c: The colour
 * int x0, int y0, int x1, int y1: The rectangle, end exclusive; clipped to the surface
 *
 * Returns int: The number of matching pixels
 */
int count(const PixelSurface& s, Color c, int x0, int y0, int x1, int y1) {
	int n = 0;
	for (int y = std::max(0, y0); y < std::min(s.height(), y1); ++y) {
		for (int x = std::max(0, x0); x < std::min(s.width(), x1); ++x) {
			n += same(at(s, x, y), c) ? 1 : 0;
		}
	}
	return n;
}

/**
 * Counts the pixels of a colour on the whole surface
 *
 * Parameters:
 * const PixelSurface& s: The surface
 * Color c: The colour
 *
 * Returns int: The number of matching pixels
 */
int countAll(const PixelSurface& s, Color c) {
	return count(s, c, 0, 0, s.width(), s.height());
}

/**
 * Average position of the pixels of a colour inside a cell
 *
 * Parameters:
 * const PixelSurface& s: The surface
 * Color c: The colour
 * SpriteCell cell: The cell
 * double& cx, double& cy: Receive the centroid
 */
void centroid(const PixelSurface& s, Color c, SpriteCell cell, double& cx, double& cy) {
	double sx = 0, sy = 0, n = 0;
	for (int y = cell.py; y < cell.py + CELL; ++y) {
		for (int x = cell.px; x < cell.px + CELL; ++x) {
			if (same(at(s, x, y), c)) {
				sx += x;
				sy += y;
				n += 1;
			}
		}
	}
	cx = n > 0 ? sx / n : -1;
	cy = n > 0 ? sy / n : -1;
}

PixelSurface freshSurface() {
	PixelSurface s(256, 256);
	s.clear(BG);
	return s;
}

}  // namespace

TEST(Sprites, HeadCentreIsTheHeadColourInEveryDirection) {
	const int sq = spriteSquare(CELL);
	const SpriteCell head{120, 120};
	for (SpriteCell neck : {SpriteCell{80, 120}, SpriteCell{160, 120}, SpriteCell{120, 80}, SpriteCell{120, 160}}) {
		PixelSurface s = freshSurface();
		drawSnake(s, {head, neck}, CELL, PALETTE);
		EXPECT_TRUE(same(at(s, head.px + sq / 2, head.py + sq / 2), PALETTE.head)) << neck.px << "," << neck.py;
	}
}

TEST(Sprites, BodyCentreIsTheBodyColour) {
	const int sq = spriteSquare(CELL);
	PixelSurface s = freshSurface();
	drawSnake(s, {SpriteCell{120, 120}, SpriteCell{80, 120}, SpriteCell{40, 120}}, CELL, PALETTE);
	// Above the diamond marked on the back, which covers the exact centre
	EXPECT_TRUE(same(at(s, 80 + sq / 2, 120 + sq / 6), PALETTE.body));
	EXPECT_TRUE(same(at(s, 40 + sq / 2, 120 + sq / 6), PALETTE.body));
	EXPECT_FALSE(same(at(s, 80 + sq / 2, 120 + sq / 2), PALETTE.body));  // the diamond is there
}

TEST(Sprites, EyesFollowTheDirection) {
	struct Case { SpriteCell neck; int sx, sy; };  // sign the eyes' centroid must have relative to the cell centre
	const SpriteCell head{120, 120};
	const double mid = 120 + spriteSquare(CELL) / 2.0;
	for (Case c : {Case{{80, 120}, 1, 0}, Case{{160, 120}, -1, 0}, Case{{120, 80}, 0, 1}, Case{{120, 160}, 0, -1}}) {
		PixelSurface s = freshSurface();
		drawSnake(s, {head, c.neck}, CELL, PALETTE);
		double cx, cy;
		centroid(s, PALETTE.eye, head, cx, cy);
		ASSERT_GT(countAll(s, PALETTE.eye), 0);
		if (c.sx != 0) {
			EXPECT_GT((cx - mid) * c.sx, 3.0) << "neck " << c.neck.px << "," << c.neck.py;
		}
		if (c.sy != 0) {
			EXPECT_GT((cy - mid) * c.sy, 3.0) << "neck " << c.neck.px << "," << c.neck.py;
		}
		EXPECT_GT(countAll(s, PALETTE.pupil), 0);
	}
}

TEST(Sprites, TongueStaysInsideTheHeadCellAndPointsForward) {
	const SpriteCell head{120, 120};
	PixelSurface s = freshSurface();
	drawSnake(s, {head, SpriteCell{80, 120}}, CELL, PALETTE);  // facing right
	const int total = countAll(s, PALETTE.tongue);
	ASSERT_GT(total, 0);
	EXPECT_EQ(count(s, PALETTE.tongue, head.px, head.py, head.px + CELL, head.py + CELL), total);
	EXPECT_EQ(count(s, PALETTE.tongue, head.px, head.py, head.px + CELL / 2, head.py + CELL), 0);  // none in the back half
}

TEST(Sprites, NoGapBetweenConsecutiveSegments) {
	const int sq = spriteSquare(CELL);
	PixelSurface s = freshSurface();
	// An L: the tail goes up from (80,160) to (80,120), then the snake turns right to the head at (120,120)
	drawSnake(s, {SpriteCell{120, 120}, SpriteCell{80, 120}, SpriteCell{80, 160}}, CELL, PALETTE);
	EXPECT_TRUE(same(at(s, 80 + sq / 2, 120 + CELL - 1), PALETTE.body));  // the gap row between the middle and the tail
	const Color between = at(s, 120 - 1, 120 + sq / 2);                      // the gap column between the middle and the head
	EXPECT_TRUE(same(between, PALETTE.body) || same(between, PALETTE.head));
}

TEST(Sprites, NoBridgeBetweenNonConsecutiveNeighbours) {
	const int sq = spriteSquare(CELL);
	PixelSurface s = freshSurface();
	// A U: the tail (80,120) ends right below the head (80,80) without being connected to it
	drawSnake(s, {SpriteCell{80, 80}, SpriteCell{120, 80}, SpriteCell{120, 120}, SpriteCell{80, 120}}, CELL, PALETTE);
	EXPECT_TRUE(same(at(s, 80 + sq / 2, 80 + CELL - 1), BG));
}

TEST(Sprites, OneAndTwoCellSnakesDrawTheirHead) {
	const int sq = spriteSquare(CELL);
	PixelSurface one = freshSurface();
	drawSnake(one, {SpriteCell{120, 120}}, CELL, PALETTE);
	EXPECT_TRUE(same(at(one, 120 + sq / 2, 120 + sq / 2), PALETTE.head));
	EXPECT_GT(countAll(one, PALETTE.eye), 0);
	PixelSurface two = freshSurface();
	drawSnake(two, {SpriteCell{120, 120}, SpriteCell{80, 120}}, CELL, PALETTE);
	EXPECT_TRUE(same(at(two, 120 + sq / 2, 120 + sq / 2), PALETTE.head));
	PixelSurface none = freshSurface();
	drawSnake(none, {}, CELL, PALETTE);
	EXPECT_EQ(countAll(none, PALETTE.head), 0);
}

TEST(Sprites, TinyCellsFallBackToSquaresAndCircle) {
	for (int cell : {7, 3, 1}) {
		const int sq = spriteSquare(cell);
		PixelSurface s(64, 64);
		s.clear(BG);
		drawSnake(s, {SpriteCell{20, 20}, SpriteCell{20 - cell, 20}}, cell, PALETTE);
		drawApple(s, SpriteCell{40, 40}, cell, PALETTE);
		EXPECT_TRUE(same(at(s, 20, 20), PALETTE.head)) << cell;
		EXPECT_EQ(count(s, PALETTE.head, 0, 0, 64, 64), sq * sq) << cell;
		EXPECT_EQ(countAll(s, PALETTE.eye), 0) << cell;
		EXPECT_EQ(countAll(s, PALETTE.tongue), 0) << cell;
		EXPECT_EQ(countAll(s, PALETTE.stem), 0) << cell;
		EXPECT_EQ(countAll(s, PALETTE.leaf), 0) << cell;
		EXPECT_TRUE(same(at(s, 40 + cell / 2, 40 + cell / 2), PALETTE.apple)) << cell;
	}
}

TEST(Sprites, SpritesPartlyOutsideTheSurfaceDoNotCrash) {
	PixelSurface s(32, 32);
	s.clear(BG);
	drawSnake(s, {SpriteCell{-10, -10}, SpriteCell{-50, -10}, SpriteCell{25, 25}}, CELL, PALETTE);
	drawApple(s, SpriteCell{20, 20}, CELL, PALETTE);
	EXPECT_EQ(s.pixelsRgb().size(), static_cast<size_t>(32 * 32 * 3));
}

TEST(Sprites, AppleCentreIsTheFoodColourWithStemAndLeaf) {
	const int sq = spriteSquare(CELL);
	PixelSurface s = freshSurface();
	drawApple(s, SpriteCell{120, 120}, CELL, PALETTE);
	EXPECT_TRUE(same(at(s, 120 + sq / 2, 120 + sq / 2), PALETTE.apple));
	EXPECT_GT(countAll(s, PALETTE.stem), 0);
	EXPECT_GT(countAll(s, PALETTE.leaf), 0);
}

TEST(Sprites, AppleStaysInsideItsCell) {
	PixelSurface s = freshSurface();
	drawApple(s, SpriteCell{120, 120}, CELL, PALETTE);
	int outside = 0;
	for (int y = 0; y < s.height(); ++y) {
		for (int x = 0; x < s.width(); ++x) {
			const bool inside = x >= 120 && x < 120 + CELL && y >= 120 && y < 120 + CELL;
			outside += (!inside && !same(at(s, x, y), BG)) ? 1 : 0;
		}
	}
	EXPECT_EQ(outside, 0);
}
