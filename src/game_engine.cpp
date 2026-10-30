/**
 * game_engine.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Class GameEngine implementation
 */
#include "game_engine.hpp"
#include <thread>
#include <stdexcept>
#include <iostream>

/**
 * Constructor for the GameEngine class
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * int rows: Number of rows in the board (default: 20)
 * int cols: Number of columns in the board (default: 20)
 * int speedMs: Game speed in milliseconds (default: 200)
 * unsigned seed: Seed for food placement
 * bool color: Whether to render with ANSI colors
 */
GameEngine::GameEngine(IKeySource& keys, const IInputHandler& input, int rows, int cols, int speedMs, unsigned seed, bool color)
	: scoreManager_()
	, state_(rows, cols, scoreManager_.getBestScore(), seed)
	, keys_(keys)
	, input_(input)
	, renderer_(std::cout, color)
	, direction_(Direction::RIGHT)
	, running_(true)
	, speedMs_(speedMs) {
}

/**
 * Reads and maps every pending key for one tick
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& handler: Maps keys to actions
 *
 * Returns TickInput: quit if any key mapped to Quit, else the last move; Restart and Confirm are ignored
 */
TickInput GameEngine::readTick(IKeySource& keys, const IInputHandler& handler) {
	TickInput tick;
	for (const Key& key : keys.poll()) {
		std::optional<Action> action = handler.handle(key);
		if (!action) {
			continue;
		}
		switch (*action) {
			case Action::MoveUp: tick.move = Direction::UP; break;
			case Action::MoveDown: tick.move = Direction::DOWN; break;
			case Action::MoveLeft: tick.move = Direction::LEFT; break;
			case Action::MoveRight: tick.move = Direction::RIGHT; break;
			case Action::Quit: tick.quit = true; break;
			case Action::Restart: break;
			case Action::Confirm: break;
			case Action::Menu: break;
		}
	}
	return tick;
}

/**
 * Discards keys typed before the prompt, shows the game over panel, then waits for restart, menu or quit
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& handler: Maps keys to actions
 * const std::function<void()>& showPanel: Draws the game over panel, called after the flush
 *
 * Returns GameEnd: Restart or Menu on those keys; Quit on Quit or EOF. Moves and unmapped keys are ignored
 */
GameEnd GameEngine::awaitChoice(IKeySource& keys, const IInputHandler& handler,
		const std::function<void()>& showPanel) {
	keys.flush();
	showPanel();
	while (true) {
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return GameEnd::Quit;
		}
		std::optional<Action> action = handler.handle(*key);
		if (action == Action::Restart) {
			return GameEnd::Restart;
		}
		if (action == Action::Menu) {
			return GameEnd::Menu;
		}
		if (action == Action::Quit) {
			return GameEnd::Quit;
		}
	}
}

/**
 * Tells whether two directions are opposite
 *
 * Parameters:
 * Direction current: The direction the snake is moving
 * Direction next: The requested direction
 *
 * Returns bool: true if next reverses current
 */
bool GameEngine::isOpposite(Direction current, Direction next) {
	switch (current) {
		case Direction::UP: return next == Direction::DOWN;
		case Direction::DOWN: return next == Direction::UP;
		case Direction::LEFT: return next == Direction::RIGHT;
		case Direction::RIGHT: return next == Direction::LEFT;
	}
	return false;
}

/**
 * Sets the game speed for the next rounds
 *
 * Parameters:
 * int speedMs: Game speed in milliseconds per frame
 */
void GameEngine::setSpeed(int speedMs) {
	speedMs_ = speedMs;
}

/**
 * Runs the main game loop, restarting on request, until the player asks for the menu or quits
 *
 * Returns GameEnd: Menu or Quit; the board is reset for the next run() after Menu
 */
GameEnd GameEngine::run() {
	GameEnd end;
	do {
		renderer_.clear();
		renderer_.render(state_);

		while (running_) {
			TickInput tick = readTick(keys_, input_);
			if (tick.quit) {
				running_ = false;
				break;
			}

			if (tick.move && !isOpposite(direction_, *tick.move)) {
				direction_ = *tick.move;
			}

			try {
				int dirInt = static_cast<int>(direction_);
				state_.moveSnake(dirInt);
			} catch (const std::runtime_error&) {
				running_ = false;
				break;
			}

			renderer_.clear();
			renderer_.render(state_);

			std::this_thread::sleep_for(std::chrono::milliseconds(speedMs_));
		}

		int finalScore = state_.getFinalScore();
		scoreManager_.saveScore(finalScore);

		end = awaitChoice(keys_, input_, [&]() {
			renderer_.printGameOver(
				finalScore,
				scoreManager_.getBestScore(),
				scoreManager_.getTotalGames(),
				scoreManager_.getAverageScore()
			);
		});

		if (end != GameEnd::Quit) {
			state_.reset();
			direction_ = Direction::RIGHT;
			running_ = true;
			renderer_.reset();
		}
	} while (end == GameEnd::Restart);
	return end;
}
