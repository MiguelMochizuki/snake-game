/**
 * game_engine.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Class GameEngine implementation
 */
#include "snake_game/game/game_engine.hpp"
#include <stdexcept>

/**
 * Constructor for the GameEngine class
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * IRenderer& renderer: Draws the game and paces it
 * IScoreStore& scores: Saves finished games and gives the best score
 * int rows: Number of rows in the board
 * int cols: Number of columns in the board
 * int speedMs: Game speed in milliseconds
 * unsigned seed: Seed for food placement
 */
GameEngine::GameEngine(IKeySource& keys, const IInputHandler& input, IRenderer& renderer, IScoreStore& scores, int rows, int cols, int speedMs, unsigned seed)
	: scores_(scores)
	, state_(rows, cols, scores_.bestScore(Level::Normal), seed)
	, keys_(keys)
	, input_(input)
	, renderer_(renderer)
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
			case Action::MoveUp: tick.moves.push_back(Direction::UP); break;
			case Action::MoveDown: tick.moves.push_back(Direction::DOWN); break;
			case Action::MoveLeft: tick.moves.push_back(Direction::LEFT); break;
			case Action::MoveRight: tick.moves.push_back(Direction::RIGHT); break;
			case Action::Quit: tick.quit = true; break;
			case Action::Restart: break;
			case Action::Confirm: break;
			case Action::Menu: break;
			case Action::Clear: break;
			case Action::Pause: tick.pause = true; break;
			case Action::AutoPause: tick.pause = true; break;
			case Action::Back: tick.pause = true; break;
		}
	}
	return tick;
}

/**
 * Shows the game over panel, discards keys typed before and while it appeared, then waits for restart, menu or quit
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
	// Flush after the panel: it may take a while to appear (the victory fade), and keys typed meanwhile must not answer it
	showPanel();
	keys.flush();
	while (true) {
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return GameEnd::Quit;
		}
		std::optional<Action> action = handler.handle(*key);
		if (action == Action::Restart) {
			return GameEnd::Restart;
		}
		if (action == Action::Menu || action == Action::Back) {
			return GameEnd::Menu;
		}
		if (action == Action::Quit) {
			return GameEnd::Quit;
		}
	}
}

bool GameEngine::awaitResume(IKeySource& keys, const IInputHandler& handler,
		const std::function<void()>& showPanel) {
	showPanel();
	keys.flush();
	while (true) {
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return false;
		}
		std::optional<Action> action = handler.handle(*key);
		if (action == Action::Pause || action == Action::Back) {
			keys.flush();
			return true;
		}
		if (action == Action::Quit) {
			return false;
		}
	}
}

void GameEngine::queueTurns(std::deque<Direction>& pending, Direction current, const std::vector<Direction>& moves) {
	for (Direction move : moves) {
		const Direction last = pending.empty() ? current : pending.back();
		if (pending.size() >= MAX_PENDING_TURNS) {
			break;
		}
		if (move == last || isOpposite(last, move)) {
			continue;
		}
		pending.push_back(move);
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
 * Sets the level and game speed for the next rounds
 *
 * Parameters:
 * Level level: Level that tags the saved scores and sets the best score shown
 * int speedMs: Game speed in milliseconds per frame
 */
void GameEngine::setLevel(Level level, int speedMs) {
	level_ = level;
	speedMs_ = speedMs;
	applyBest();
}

/**
 * Sets the best score the status line starts from: the saved one, or 0 for a practice session, whose best is its own
 */
void GameEngine::applyBest() {
	state_.setBestScore(practice_ ? 0 : scores_.bestScore(level_));
}

/**
 * Sets the sound effects
 *
 * Parameters:
 * ISound* sound: Receives eat and game over events, must outlive the engine; nullptr for silence
 */
void GameEngine::setSound(ISound* sound) {
	sound_ = sound;
}

/**
 * Turns practice mode on or off
 *
 * Parameters:
 * bool practice: true to play without saving scores to the leaderboard
 */
void GameEngine::setPractice(bool practice) {
	practice_ = practice;
	applyBest();
}

/**
 * Runs the main game loop, restarting on request, until the player asks for the menu or quits
 *
 * Returns GameEnd: Menu or Quit; the board is reset for the next run() after Menu
 */
GameEnd GameEngine::run() {
	GameEnd end;
	renderer_.setPractice(practice_);
	do {
		renderer_.clear();
		renderer_.render(state_);
		if (sound_) {
			sound_->musicStart();
		}

		// A game the player walks away from (Q, Ctrl+C, closing the window) is not a finished game
		bool abandoned = false;
		while (running_) {
			TickInput tick = readTick(keys_, input_);
			if (tick.quit) {
				abandoned = true;
				running_ = false;
				break;
			}

			if (tick.pause) {
				if (sound_) {
					sound_->musicStop();
				}
				if (!awaitResume(keys_, input_, [&]() { renderer_.printPaused(); })) {
					abandoned = true;
					running_ = false;
					break;
				}
				if (sound_) {
					sound_->musicStart();
				}
				// The pause panel took the whole screen, so the board is drawn again from scratch
				pending_.clear();
				renderer_.reset();
				renderer_.clear();
				renderer_.render(state_);
				continue;
			}

			queueTurns(pending_, direction_, tick.moves);
			if (!pending_.empty()) {
				direction_ = pending_.front();
				pending_.pop_front();
			}

			const int scoreBefore = state_.getScore();
			try {
				int dirInt = static_cast<int>(direction_);
				state_.moveSnake(dirInt);
			} catch (const std::runtime_error&) {
				running_ = false;
				break;
			}

			renderer_.clear();
			renderer_.render(state_);
			if (sound_ && state_.getScore() > scoreBefore) {
				sound_->eat();
			}
			if (state_.isWon()) {
				running_ = false;
				break;
			}

			renderer_.pause(speedMs_);
		}

		int finalScore = state_.getFinalScore();
		if (!practice_ && !abandoned) {
			scores_.saveScore(level_, finalScore);
		}

		// Practice has no saved figures: the panel gets this session's own best (kept across restarts) and no games count
		const ScoreStats panelStats = practice_ ? ScoreStats{state_.getBestScore(), 0} : scores_.stats(level_);
		const bool won = state_.isWon();
		end = awaitChoice(keys_, input_, [&]() {
			if (sound_) {
				sound_->musicStop();
				if (won) {
					sound_->win();
				} else {
					sound_->gameOver();
				}
			}
			if (won) {
				renderer_.printVictory(level_, finalScore, panelStats);
			} else {
				renderer_.printGameOver(level_, finalScore, panelStats);
			}
		});

		if (end != GameEnd::Quit) {
			state_.reset();
			direction_ = Direction::RIGHT;
			pending_.clear();
			running_ = true;
			renderer_.reset();
		}
	} while (end == GameEnd::Restart);
	return end;
}
