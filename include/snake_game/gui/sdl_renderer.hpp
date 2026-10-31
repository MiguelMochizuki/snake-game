/**
 * sdl_renderer.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Graphical renderer of the game board, HUD and game over panel
 */
#ifndef SDL_RENDERER_HPP
#define SDL_RENDERER_HPP

#include "snake_game/game/game_state.hpp"
#include "snake_game/gui/canvas.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/render/renderer.hpp"
#include <functional>
#include <optional>

/**
 * Where the board sits on the canvas
 *
 * Attributes:
 * int cell: Cell side in pixels, at least 1
 * int originX, int originY: Top-left corner of the board including its wall frame
 */
struct Layout {
	int cell;
	int originX;
	int originY;
};

/**
 * Largest whole-pixel cell that fits the board with its wall frame and the HUD
 *
 * Parameters:
 * int canvasW, int canvasH: Canvas size in pixels
 * int cols, int rows: Board size in cells, without the wall frame
 * int hudHeight: Pixels reserved below the board
 *
 * Returns int: The cell side, never below 1
 */
int cellSizeFor(int canvasW, int canvasH, int cols, int rows, int hudHeight);

/**
 * Places the board centred horizontally with the HUD below it
 *
 * Parameters:
 * int canvasW, int canvasH: Canvas size in pixels
 * int cols, int rows: Board size in cells, without the wall frame
 *
 * Returns Layout: Cell size and origin
 */
Layout layoutFor(int canvasW, int canvasH, int cols, int rows);

/**
 * Draws the game on a Canvas and paces it by handling window events
 *
 * Attributes:
 * Canvas& canvas_: Where to draw
 * SdlApp& app_: Event pump used while waiting and for redraws on resize
 * std::optional<GameState> last_: The last state drawn
 * std::function<void()> panel_: Draws the game over panel, empty while a game runs
 */
class SdlRenderer : public IRenderer {
	Canvas& canvas_;
	SdlApp& app_;
	std::optional<GameState> last_;
	std::function<void()> panel_;

	void drawBoard(const GameState& state);
	void redrawAll();

public:
	/**
	 * Constructor for the SdlRenderer class
	 *
	 * Parameters:
	 * Canvas& canvas: Where to draw, must outlive the renderer
	 * SdlApp& app: Event pump, must outlive the renderer
	 */
	SdlRenderer(Canvas& canvas, SdlApp& app);

	void reset() override;
	void clear() override;
	void render(const GameState& state) override;
	void printGameOver(Level level, int finalScore, const ScoreStats& stats) override;
	void pause(int ms) override;

	/**
	 * Returns GameState: A copy of the last state drawn (a default board before any)
	 */
	GameState lastState() const;
};

#endif
