/**
 * sdl_keys_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the SDL key mapping, event pump and key source; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <SDL3/SDL.h>

#include "snake_game/gui/canvas.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/gui/sdl_key_source.hpp"

namespace {

/**
 * Queues a key-down event as the window system would
 *
 * Parameters:
 * SDL_Keycode code: The key
 */
void pushKey(SDL_Keycode code) {
	SDL_Event e;
	SDL_zero(e);
	e.type = SDL_EVENT_KEY_DOWN;
	e.key.key = code;
	SDL_PushEvent(&e);
}

/**
 * Queues an event with no payload
 *
 * Parameters:
 * SDL_EventType type: The event type
 */
void pushEvent(SDL_EventType type) {
	SDL_Event e;
	SDL_zero(e);
	e.type = type;
	SDL_PushEvent(&e);
}

}  // namespace

TEST(SdlKeys, ArrowsMapToDirections) {
	EXPECT_EQ(keyFromSdl(SDLK_UP), specialKey(KeyKind::Up));
	EXPECT_EQ(keyFromSdl(SDLK_DOWN), specialKey(KeyKind::Down));
	EXPECT_EQ(keyFromSdl(SDLK_LEFT), specialKey(KeyKind::Left));
	EXPECT_EQ(keyFromSdl(SDLK_RIGHT), specialKey(KeyKind::Right));
}

TEST(SdlKeys, LettersMapToLowerCaseChars) {
	EXPECT_EQ(keyFromSdl(SDLK_W), charKey('w'));
	EXPECT_EQ(keyFromSdl(SDLK_Q), charKey('q'));
}

TEST(SdlKeys, EnterAndKeypadEnterMapToEnter) {
	EXPECT_EQ(keyFromSdl(SDLK_RETURN), specialKey(KeyKind::Enter));
	EXPECT_EQ(keyFromSdl(SDLK_KP_ENTER), specialKey(KeyKind::Enter));
}

TEST(SdlKeys, UnmappedKeyIsNullopt) {
	EXPECT_FALSE(keyFromSdl(SDLK_F1).has_value());
	EXPECT_FALSE(keyFromSdl(SDLK_LSHIFT).has_value());
}

TEST(SdlKeys, NonInteractiveAppNeverBlocks) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(32, 32);
	SdlApp app(*canvas, false);
	app.pump(0);
	app.pump(-1);
	EXPECT_TRUE(app.keys().empty());
	SdlKeySource source(app);
	EXPECT_FALSE(source.waitKey().has_value());
}

TEST(SdlKeys, PauseKeepsKeysForPoll) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(32, 32);
	SdlApp app(*canvas, true);
	SdlKeySource source(app);
	source.flush();
	pushKey(SDLK_A);
	app.pump(20);  // what SdlRenderer::pause does
	std::vector<Key> keys = source.poll();
	ASSERT_EQ(keys.size(), 1u);
	EXPECT_EQ(keys[0], charKey('a'));
	EXPECT_TRUE(source.poll().empty());
}

TEST(SdlKeys, WaitKeyReturnsQueuedKey) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(32, 32);
	SdlApp app(*canvas, true);
	SdlKeySource source(app);
	source.flush();
	pushKey(SDLK_RETURN);
	std::optional<Key> key = source.waitKey();
	ASSERT_TRUE(key.has_value());
	EXPECT_EQ(*key, specialKey(KeyKind::Enter));
}

TEST(SdlKeys, WindowCloseBecomesQ) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(32, 32);
	SdlApp app(*canvas, true);
	SdlKeySource source(app);
	source.flush();
	pushEvent(SDL_EVENT_QUIT);
	std::optional<Key> key = source.waitKey();
	ASSERT_TRUE(key.has_value());
	EXPECT_EQ(*key, charKey('q'));
}

TEST(SdlKeys, ResizeRedrawsTheScene) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(32, 32);
	SdlApp app(*canvas, true);
	int redraws = 0;
	app.setScene([&] { ++redraws; });
	pushEvent(SDL_EVENT_WINDOW_RESIZED);
	app.pump(0);
	EXPECT_EQ(redraws, 1);
}
