/**
 * sdl_menu_view.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Graphical start menu, leaderboard and help screens
 */
#ifndef SDL_MENU_VIEW_HPP
#define SDL_MENU_VIEW_HPP

#include "snake_game/gui/draw_surface.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/menu/menu_view.hpp"

/**
 * Draws the menu screens on a surface with the same texts as the terminal menu
 *
 * Attributes:
 * IDrawSurface& canvas_: Where to draw
 * SdlApp& app_: Event pump, told how to redraw the screen on resize
 */
class SdlMenuView : public IMenuView {
	IDrawSurface& canvas_;
	SdlApp& app_;

public:
	/**
	 * Constructor for the SdlMenuView class
	 *
	 * Parameters:
	 * IDrawSurface& canvas: Where to draw, must outlive the view
	 * SdlApp& app: Event pump, must outlive the view
	 */
	SdlMenuView(IDrawSurface& canvas, SdlApp& app);

	void show(const MenuSnapshot& snapshot) override;
	void showLeaderboard(const LeaderboardSnapshot& snapshot) override;
	void showHelp() override;
	void showConfirmClear(Level level) override;
};

#endif
