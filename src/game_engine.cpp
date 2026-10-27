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
 * Returns TickInput: quit if any key mapped to Quit, else the last move; Restart is ignored
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
		}
	}
	return tick;
}

/**
 * Discards keys typed before the prompt, then waits on the game over screen for restart or quit
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& handler: Maps keys to actions
 *
 * Returns bool: true on Restart; false on Quit or EOF. Moves and unmapped keys are ignored
 */
bool GameEngine::awaitRestart(IKeySource& keys, const IInputHandler& handler) {
	keys.flush();
	while (true) {
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return false;
		}
		std::optional<Action> action = handler.handle(*key);
		if (action == Action::Restart) {
			return true;
		}
		if (action == Action::Quit) {
			return false;
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
 * Runs the main game loop
 */
void GameEngine::run() {
	bool restart;
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

		renderer_.printGameOver(
			finalScore,
			scoreManager_.getBestScore(),
			scoreManager_.getTotalGames(),
			scoreManager_.getAverageScore()
		);
		restart = awaitRestart(keys_, input_);

		if (restart) {
			state_.reset();
			direction_ = Direction::RIGHT;
			running_ = true;
			renderer_.reset();
		}
	} while (restart);
}
