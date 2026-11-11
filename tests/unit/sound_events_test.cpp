/**
 * sound_events_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the music and menu-blip events of ISound
 */
#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "engine_fakes.hpp"
#include "snake_game/audio/synth.hpp"
#include "snake_game/audio/synth_sound.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/menu/menu_controller.hpp"
#include "snake_game/render/sound.hpp"

using namespace fakes;

namespace {

/**
 * Sound that records every event by name
 *
 * Attributes:
 * std::vector<std::string> events: Event names in order
 */
class RecordingSound : public ISound {
public:
	std::vector<std::string> events;

	void eat() override { events.push_back("eat"); }
	void gameOver() override { events.push_back("gameOver"); }
	void musicStart() override { events.push_back("musicStart"); }
	void musicStop() override { events.push_back("musicStop"); }
	void musicFinale() override { events.push_back("musicFinale"); }
	void win() override { events.push_back("win"); }
	void menuMove() override { events.push_back("menuMove"); }
	void menuSelect() override { events.push_back("menuSelect"); }
};

/**
 * Menu view that draws nothing
 */
class SilentView : public IMenuView {
public:
	void show(const MenuSnapshot&) override {}
	void showLeaderboard(const LeaderboardSnapshot&) override {}
	void showHelp() override {}
};

/**
 * Score source with fixed empty figures
 */
class EmptyScores : public IScoreSource {
public:
	ScoreStats stats(Level) const override { return ScoreStats{}; }
	std::vector<int> topScores(Level, size_t) const override { return {}; }
};

using Events = std::vector<std::string>;

}  // namespace

TEST(SoundEvents, EngineStartsMusicOnRunAndEndsWithTheFinaleBeforeGameOver) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	RecordingSound sound;
	GameEngine engine(keys, input, renderer, 20, 20, 0, 42, dir.path.string());
	engine.setSound(&sound);

	engine.run();

	EXPECT_EQ(sound.events, (Events{"musicStart", "musicFinale", "gameOver", "musicStop"}));
}

TEST(SoundEvents, MusicRestartsOnRestart) {
	TempDir dir;
	ScriptedKeys keys;
	keys.waits.push_back(charKey('r'));
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	RecordingSound sound;
	GameEngine engine(keys, input, renderer, 20, 20, 0, 42, dir.path.string());
	engine.setSound(&sound);

	engine.run();

	EXPECT_EQ(sound.events, (Events{"musicStart", "musicFinale", "gameOver", "musicStart", "musicFinale", "gameOver", "musicStop"}));
}

TEST(SoundEvents, WinningPlaysTheFinaleAndTheWinCue) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	RecordingSound sound;
	GameEngine engine(keys, input, renderer, 1, 4, 0, 5, dir.path.string());  // seed 5: the first move eats the last food
	engine.setSound(&sound);

	engine.run();

	EXPECT_EQ(sound.events, (Events{"musicStart", "eat", "musicFinale", "win", "musicStop"}));
}

namespace {

/**
 * Sound that only implements the required cues, to check the defaults of the others
 */
class BareSound : public ISound {
public:
	std::vector<std::string> events;

	void eat() override {}
	void gameOver() override { events.push_back("gameOver"); }
	void musicStop() override { events.push_back("musicStop"); }
};

}  // namespace

TEST(SoundEvents, DefaultWinIsGameOverAndDefaultFinaleIsStop) {
	BareSound sound;
	sound.win();
	sound.musicFinale();
	EXPECT_EQ(sound.events, (Events{"gameOver", "musicStop"}));
}

TEST(SoundEvents, SynthSoundWinMakesASound) {
	Synth synth(44100);
	SynthSound sound(synth);
	sound.win();
	std::vector<float> out(2000);
	synth.render(out.data(), 2000);
	EXPECT_NE(*std::max_element(out.begin(), out.end()), 0.0f);
}

TEST(SoundEvents, NullSoundAndBellIgnoreNewMethods) {
	std::ostringstream os;
	TerminalBell bell(os);
	bell.musicStart();
	bell.musicStop();
	bell.menuMove();
	bell.menuSelect();
	EXPECT_EQ(os.str(), "");
	NullSound quiet;
	quiet.musicStart();
	quiet.menuSelect();
}

TEST(SoundEvents, SynthSoundForwardsToSynth) {
	Synth synth(44100);
	SynthSound sound(synth);
	std::vector<float> out(2000);
	sound.menuSelect();
	synth.render(out.data(), 2000);
	EXPECT_NE(out[0], 0.0f);
	sound.musicStart();
	synth.render(out.data(), 2000);
	EXPECT_NE(out[100], 0.0f);
	sound.musicStop();
	synth.render(out.data(), 2000);
	EXPECT_EQ(out[100], 0.0f);
}

TEST(SoundEvents, MenuBlipsMoveThenSelect) {
	ScriptedKeys keys;
	keys.waits = {specialKey(KeyKind::Down), specialKey(KeyKind::Down), specialKey(KeyKind::Enter)};
	CompositeInputHandler input = makeInput();
	SilentView view;
	MenuModel model;
	EmptyScores scores;
	RecordingSound sound;

	runMenu(keys, input, view, model, scores, &sound);

	EXPECT_EQ(sound.events, (Events{"menuMove", "menuMove", "menuSelect"}));
}

TEST(SoundEvents, MenuBlipsFollowSoundSwitch) {
	ScriptedKeys keys;
	// Four Downs reach Sound, Enter turns it Off, then Up and Enter must stay silent
	keys.waits = {specialKey(KeyKind::Down), specialKey(KeyKind::Down), specialKey(KeyKind::Down), specialKey(KeyKind::Down),
		specialKey(KeyKind::Enter), specialKey(KeyKind::Up), specialKey(KeyKind::Down)};
	CompositeInputHandler input = makeInput();
	SilentView view;
	MenuModel model;
	EmptyScores scores;
	RecordingSound sound;

	runMenu(keys, input, view, model, scores, &sound);

	EXPECT_FALSE(model.sound());
	EXPECT_EQ(sound.events, (Events{"menuMove", "menuMove", "menuMove", "menuMove"}));
}
