/**
 * sdl_menu_view.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Graphical start menu, leaderboard and help screens
 */
#ifndef SDL_MENU_VIEW_HPP
#define SDL_MENU_VIEW_HPP

#include "snake_game/gui/canvas.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/menu/menu_view.hpp"

/**
 * Draws the menu screens on a Canvas with the same texts as the terminal menu
 *
 * Attributes:
 * Canvas& canvas_: Where to draw
 * SdlApp& app_: Event pump, told how to redraw the screen on resize
 */
class SdlMenuView : public IMenuView {
	Canvas& canvas_;
	SdlApp& app_;

public:
	/**
	 * Constructor for the SdlMenuView class
	 *
	 * Parameters:
	 * Canvas& canvas: Where to draw, must outlive the view
	 * SdlApp& app: Event pump, must outlive the view
	 */
	SdlMenuView(Canvas& canvas, SdlApp& app);

	void show(const MenuSnapshot& snapshot) override;
	void showLeaderboard(const LeaderboardSnapshot& snapshot) override;
	void showHelp() override;
};

#endif
