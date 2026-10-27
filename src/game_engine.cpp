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
 * int rows: Number of rows in the board (default: 20)
 * int cols: Number of columns in the board (default: 20)
 * int speedMs: Game speed in milliseconds (default: 200)
 * unsigned seed: Seed for food placement
 * bool color: Whether to render with ANSI colors
 */
GameEngine::GameEngine(int rows, int cols, int speedMs, unsigned seed, bool color)
	: scoreManager_()
	, state_(rows, cols, scoreManager_.getBestScore(), seed)
	, inputHandler_()
	, renderer_(std::cout, color)
	, direction_(Direction::RIGHT)
	, running_(true)
	, speedMs_(speedMs) {
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
			inputHandler_.processInput();

			if (inputHandler_.shouldQuit()) {
				running_ = false;
				break;
			}

			if (inputHandler_.directionChanged()) {
				Direction newDir = inputHandler_.getDirection();

				bool isOpposite = false;
				switch (direction_) {
					case Direction::UP: isOpposite = (newDir == Direction::DOWN); break;
					case Direction::DOWN: isOpposite = (newDir == Direction::UP); break;
					case Direction::LEFT: isOpposite = (newDir == Direction::RIGHT); break;
					case Direction::RIGHT: isOpposite = (newDir == Direction::LEFT); break;
				}

				if (!isOpposite) {
					direction_ = newDir;
				}

				inputHandler_.resetDirectionChanged();
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

		restart = renderer_.showGameOver(
			finalScore,
			scoreManager_.getBestScore(),
			scoreManager_.getTotalGames(),
			scoreManager_.getAverageScore(),
			inputHandler_
		);

		if (restart) {
			state_.reset();
			direction_ = Direction::RIGHT;
			running_ = true;
			inputHandler_.resetDirectionChanged();
			inputHandler_.resetQuit();
			renderer_.reset();
		}
	} while (restart);
}
