/**
 * menu_model_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for MenuModel
 */
#include <gtest/gtest.h>

#include "menu_model.hpp"

TEST(MenuModel, StartsOnStartWithNormalLevel) {
	MenuModel m;
	EXPECT_EQ(m.selected(), MenuItem::Start);
	EXPECT_EQ(m.level(), Level::Normal);
	EXPECT_EQ(m.speedMs(), 200);
}

TEST(MenuModel, NavigationWraps) {
	MenuModel m;
	EXPECT_EQ(m.apply(Action::MoveUp), MenuOutcome::None);
	EXPECT_EQ(m.selected(), MenuItem::Quit);
	m.apply(Action::MoveDown);
	EXPECT_EQ(m.selected(), MenuItem::Start);
	m.apply(Action::MoveDown);
	EXPECT_EQ(m.selected(), MenuItem::Level);
	m.apply(Action::MoveDown);
	m.apply(Action::MoveDown);
	EXPECT_EQ(m.selected(), MenuItem::Start);
}

TEST(MenuModel, LevelCyclesBothWaysOnLevelRow) {
	MenuModel m;
	m.apply(Action::MoveDown);
	m.apply(Action::MoveRight);
	EXPECT_EQ(m.level(), Level::Hard);
	EXPECT_EQ(m.speedMs(), 100);
	m.apply(Action::MoveRight);
	EXPECT_EQ(m.level(), Level::Easy);
	EXPECT_EQ(m.speedMs(), 300);
	m.apply(Action::MoveRight);
	EXPECT_EQ(m.level(), Level::Normal);
	m.apply(Action::MoveLeft);
	EXPECT_EQ(m.level(), Level::Easy);
	m.apply(Action::MoveLeft);
	EXPECT_EQ(m.level(), Level::Hard);
}

TEST(MenuModel, LeftRightIgnoredOffLevelRow) {
	MenuModel m;
	EXPECT_EQ(m.apply(Action::MoveRight), MenuOutcome::None);
	EXPECT_EQ(m.level(), Level::Normal);
	m.apply(Action::MoveUp);
	EXPECT_EQ(m.apply(Action::MoveLeft), MenuOutcome::None);
	EXPECT_EQ(m.level(), Level::Normal);
}

TEST(MenuModel, ConfirmOutcomes) {
	MenuModel m;
	EXPECT_EQ(m.apply(Action::Confirm), MenuOutcome::Start);
	m.apply(Action::MoveDown);
	EXPECT_EQ(m.apply(Action::Confirm), MenuOutcome::None);
	m.apply(Action::MoveDown);
	EXPECT_EQ(m.apply(Action::Confirm), MenuOutcome::Quit);
}

TEST(MenuModel, QuitActionQuitsFromAnyRow) {
	MenuModel m;
	EXPECT_EQ(m.apply(Action::Quit), MenuOutcome::Quit);
	m.apply(Action::MoveDown);
	EXPECT_EQ(m.apply(Action::Quit), MenuOutcome::Quit);
}

TEST(MenuModel, RestartIsIgnored) {
	MenuModel m;
	EXPECT_EQ(m.apply(Action::Restart), MenuOutcome::None);
}

TEST(MenuModel, LevelNames) {
	EXPECT_STREQ(levelName(Level::Easy), "Easy");
	EXPECT_STREQ(levelName(Level::Normal), "Normal");
	EXPECT_STREQ(levelName(Level::Hard), "Hard");
}
