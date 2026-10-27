/**
 * input_handlers_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for the IInputHandler implementations (arrow, command, composite)
 */
#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <vector>

#include "arrow_input_handler.hpp"
#include "command_input_handler.hpp"
#include "composite_input_handler.hpp"
#include "input_handler.hpp"

/**
 * Test-only handler that maps one key to one action and ignores everything else
 *
 * Attributes:
 * Key key_: The only key this stub reacts to
 * Action action_: The action returned for key_
 */
class StubHandler : public IInputHandler {
private:
	Key key_;
	Action action_;

public:
	/**
	 * Constructor for the StubHandler class
	 *
	 * Parameters:
	 * Key key: The key to react to
	 * Action action: The action returned when key is handled
	 */
	StubHandler(Key key, Action action) : key_(key), action_(action) {}

	/**
	 * Returns the stub's action when the key matches, nullopt otherwise
	 *
	 * Parameters:
	 * const Key& key: The key to handle
	 *
	 * Returns std::optional<Action>: The action for key_, or nullopt
	 */
	std::optional<Action> handle(const Key& key) const override {
		if (key == key_) {
			return action_;
		}
		return std::nullopt;
	}
};

/**
 * Every plain character from 0 to 255, as keys
 *
 * Returns std::vector<Key>: One charKey per byte value
 */
static std::vector<Key> allCharKeys() {
	std::vector<Key> keys;
	for (int c = 0; c < 256; ++c) {
		keys.push_back(charKey(static_cast<char>(c)));
	}
	return keys;
}

TEST(ArrowInputHandler, ArrowMapsOnlyArrows) {
	ArrowInputHandler h;
	EXPECT_EQ(h.handle(specialKey(KeyKind::Up)), Action::MoveUp);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Down)), Action::MoveDown);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Left)), Action::MoveLeft);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Right)), Action::MoveRight);

	for (const Key& k : allCharKeys()) {
		EXPECT_EQ(h.handle(k), std::nullopt);
	}
	EXPECT_EQ(h.handle(specialKey(KeyKind::CtrlC)), std::nullopt);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Unknown)), std::nullopt);
}

TEST(CommandInputHandler, CommandMapsOnlyCommands) {
	CommandInputHandler h;
	EXPECT_EQ(h.handle(charKey('q')), Action::Quit);
	EXPECT_EQ(h.handle(charKey('Q')), Action::Quit);
	EXPECT_EQ(h.handle(specialKey(KeyKind::CtrlC)), Action::Quit);
	EXPECT_EQ(h.handle(charKey('r')), Action::Restart);
	EXPECT_EQ(h.handle(charKey('R')), Action::Restart);

	for (const Key& k : allCharKeys()) {
		if (k == charKey('q') || k == charKey('Q') || k == charKey('r') || k == charKey('R')) {
			continue;
		}
		EXPECT_EQ(h.handle(k), std::nullopt);
	}
	EXPECT_EQ(h.handle(specialKey(KeyKind::Up)), std::nullopt);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Down)), std::nullopt);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Left)), std::nullopt);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Right)), std::nullopt);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Unknown)), std::nullopt);
}

TEST(CompositeInputHandler, CompositeEmptyReturnsNullopt) {
	CompositeInputHandler h;
	EXPECT_EQ(h.handle(charKey('q')), std::nullopt);
	EXPECT_EQ(h.handle(specialKey(KeyKind::Up)), std::nullopt);
}

TEST(CompositeInputHandler, CompositeFirstMatchWins) {
	CompositeInputHandler h;
	h.add(std::make_unique<StubHandler>(charKey('x'), Action::Quit));
	h.add(std::make_unique<StubHandler>(charKey('x'), Action::Restart));
	EXPECT_EQ(h.handle(charKey('x')), Action::Quit);
}

TEST(CompositeInputHandler, CompositeFallsThrough) {
	CompositeInputHandler h;
	h.add(std::make_unique<ArrowInputHandler>());
	h.add(std::make_unique<CommandInputHandler>());
	EXPECT_EQ(h.handle(specialKey(KeyKind::Left)), Action::MoveLeft);
	EXPECT_EQ(h.handle(charKey('q')), Action::Quit);
	EXPECT_EQ(h.handle(charKey('z')), std::nullopt);
}
