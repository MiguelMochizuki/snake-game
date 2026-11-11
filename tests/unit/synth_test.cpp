/**
 * synth_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the chord sequencer and blip synth
 */
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <thread>
#include <vector>

#include "snake_game/audio/score.hpp"
#include "snake_game/audio/synth.hpp"

namespace {

const int RATE = 44100;
const int LOW_RATE = 2000;  // keeps whole-piece renders small: a sixteenth is 454 samples at 66 BPM
const int LOW_STEP = 454;

std::vector<float> renderFrames(Synth& s, int frames) {
	std::vector<float> out(frames);
	s.render(out.data(), frames);
	return out;
}

bool allZero(const std::vector<float>& v, size_t from = 0, size_t to = SIZE_MAX) {
	for (size_t i = from; i < v.size() && i < to; ++i) {
		if (v[i] != 0.0f) {
			return false;
		}
	}
	return true;
}

int signChanges(const std::vector<float>& v, size_t n) {
	int count = 0;
	for (size_t i = 1; i < n; ++i) {
		if ((v[i] > 0) != (v[i - 1] > 0)) {
			++count;
		}
	}
	return count;
}

}  // namespace

TEST(Score, PreludeOpensLikeBach) {
	// Bar 1 of the C major prelude: C E G C E G C E, twice
	const int bar1[8] = {60, 64, 67, 72, 76, 67, 72, 76};
	for (int i = 0; i < 16; ++i) {
		EXPECT_EQ(arpeggioNote(i), bar1[i % 8]) << "step " << i;
	}
	EXPECT_EQ(arpeggioNote(16 + 1), 62);  // bar 2 is Dm7/C: C D A D F ...
	EXPECT_EQ(arpeggioNote(32), 59);      // bar 3 is G7/B: B D G D F ...
}

TEST(Score, MelodyWaitsFourBarsThenSings) {
	for (int step = 0; step < 64; ++step) {
		EXPECT_EQ(melodyNote(step), -1) << "step " << step;
	}
	EXPECT_EQ(melodyNote(64), 76);       // E5, first note of the melody
	EXPECT_EQ(melodyNote(64 + 15), 76);  // held for the whole bar
	EXPECT_EQ(melodyNote(80), 77);       // F5
	EXPECT_EQ(melodyNote(96), 79);       // G5
	EXPECT_EQ(melodyNote(96 + 12), 74);  // D5
}

TEST(Score, DataIsConsistent) {
	for (int step = 0; step < ARPEGGIO_STEPS; ++step) {
		EXPECT_GE(arpeggioNote(step), 36) << "step " << step;
		EXPECT_LE(arpeggioNote(step), 96) << "step " << step;
	}
	EXPECT_EQ(arpeggioNote(ARPEGGIO_STEPS), -1);  // the final chord takes over
	int end = 0;
	for (int i = 0; i < MELODY_COUNT; ++i) {
		EXPECT_GE(MELODY[i].start, end) << "note " << i << " overlaps the previous one";
		EXPECT_GT(MELODY[i].length, 0);
		EXPECT_GE(MELODY[i].pitch, 48);
		EXPECT_LE(MELODY[i].pitch, 96);
		end = MELODY[i].start + MELODY[i].length;
	}
	EXPECT_LE(end, SCORE_STEPS - 8) << "melody must leave a breath before the loop restarts";
}

TEST(Score, WrapsAroundAtTheEnd) {
	for (int step : {0, 1, 17, 64, 100, 700}) {
		EXPECT_EQ(arpeggioNote(step + SCORE_STEPS), arpeggioNote(step));
		EXPECT_EQ(melodyNote(step + SCORE_STEPS), melodyNote(step));
	}
}

TEST(Score, TempoIsCalm) {
	EXPECT_LE(TEMPO_BPM, 72.0);
	EXPECT_GE(TEMPO_BPM, 54.0);
}

TEST(Synth, PlaysThroughAndRestartsSeamlessly) {
	Synth s(LOW_RATE);
	s.startMusic();
	const int loopFrames = SCORE_STEPS * LOW_STEP;
	std::vector<float> first = renderFrames(s, loopFrames);
	std::vector<float> second = renderFrames(s, loopFrames);
	EXPECT_FALSE(allZero(first));
	EXPECT_EQ(first, second);
}

TEST(Synth, LeavesABreathBeforeRestarting) {
	Synth s(LOW_RATE);
	s.startMusic();
	std::vector<float> loop = renderFrames(s, SCORE_STEPS * LOW_STEP);
	EXPECT_TRUE(allZero(loop, static_cast<size_t>((SCORE_STEPS - 8) * LOW_STEP)));
}

TEST(Synth, MelodyAddsToTheAccompaniment) {
	// Bars 1-4 are accompaniment only; the melody enters in bar 5 and the sound gets louder
	Synth s(LOW_RATE);
	s.startMusic();
	std::vector<float> out = renderFrames(s, 8 * 16 * LOW_STEP);
	double intro = 0, withMelody = 0;
	for (size_t i = 0; i < out.size() / 2; ++i) {
		intro += std::fabs(out[i]);
		withMelody += std::fabs(out[i + out.size() / 2]);
	}
	EXPECT_GT(withMelody, intro * 1.1);
}

TEST(Synth, MusicStaysQuietAndFinite) {
	Synth s(LOW_RATE);
	s.startMusic();
	float peak = 0.0f;
	for (float v : renderFrames(s, SCORE_STEPS * LOW_STEP)) {
		ASSERT_TRUE(std::isfinite(v));
		peak = std::max(peak, std::fabs(v));
	}
	EXPECT_GT(peak, 0.05f);
	EXPECT_LT(peak, 0.5f) << "leaves headroom for the blips on top";
}

TEST(Synth, SilentWhenStopped) {
	Synth s(RATE);
	EXPECT_TRUE(allZero(renderFrames(s, 5000)));
	s.startMusic();
	EXPECT_FALSE(allZero(renderFrames(s, 5000)));
	s.stopMusic();
	EXPECT_TRUE(allZero(renderFrames(s, 5000)));
}

TEST(Synth, FinalePlaysTheLastBarsOnceAndStops) {
	Synth s(LOW_RATE);
	s.startMusic();
	renderFrames(s, 3 * LOW_STEP);
	s.finale();
	const int len = (FINALE_END_STEP - FINALE_START_STEP) * LOW_STEP;
	std::vector<float> out = renderFrames(s, 2 * len);
	EXPECT_FALSE(allZero(out, 0, LOW_STEP)) << "the arpeggio of the last bars starts at once";
	EXPECT_FALSE(allZero(out, len - 4 * LOW_STEP, len - LOW_STEP)) << "the final chord is still ringing";
	EXPECT_TRUE(allZero(out, len + LOW_STEP)) << "it does not loop";
}

TEST(Synth, FinaleWorksWhenTheMusicWasOff) {
	Synth s(LOW_RATE);
	s.finale();
	EXPECT_FALSE(allZero(renderFrames(s, LOW_STEP)));
}

TEST(Synth, StartingTheMusicDuringTheFinaleStartsOver) {
	Synth a(LOW_RATE), b(LOW_RATE);
	a.startMusic();
	renderFrames(a, 5 * LOW_STEP);
	a.finale();
	renderFrames(a, 3 * LOW_STEP);
	a.startMusic();
	b.startMusic();
	EXPECT_EQ(renderFrames(a, 4 * LOW_STEP), renderFrames(b, 4 * LOW_STEP));
}

TEST(Synth, MusicLoopsAgainAfterAFinale) {
	Synth s(LOW_RATE);
	s.startMusic();
	s.finale();
	renderFrames(s, (FINALE_END_STEP - FINALE_START_STEP + 2) * LOW_STEP);
	s.startMusic();
	std::vector<float> out = renderFrames(s, (SCORE_STEPS + 5) * LOW_STEP);
	EXPECT_FALSE(allZero(out, 0, 5 * LOW_STEP)) << "a plain start plays the piece from its beginning";
	EXPECT_FALSE(allZero(out, static_cast<size_t>(SCORE_STEPS) * LOW_STEP, out.size())) << "and loops as before";
}

TEST(Synth, TheLastCommandWinsWhenSeveralArriveBetweenCallbacks) {
	Synth restarted(LOW_RATE), fresh(LOW_RATE);
	restarted.startMusic();
	renderFrames(restarted, 5 * LOW_STEP);
	restarted.finale();
	restarted.startMusic();  // R pressed right after the ending was requested
	fresh.startMusic();
	const int span = (FINALE_END_STEP - FINALE_START_STEP + 8) * LOW_STEP;
	std::vector<float> a = renderFrames(restarted, span);
	EXPECT_EQ(a, renderFrames(fresh, span)) << "the new game plays as if no ending had been asked for";

	Synth stopped(LOW_RATE);
	stopped.startMusic();
	renderFrames(stopped, 5 * LOW_STEP);
	stopped.finale();
	stopped.stopMusic();  // leaving for the menu right after
	EXPECT_TRUE(allZero(renderFrames(stopped, 4 * LOW_STEP)));
}

TEST(Synth, StartingAfterTheEndingFinishedPlaysTheNextGame) {
	Synth s(LOW_RATE);
	s.startMusic();
	s.finale();
	// One callback long enough to hold the whole ending, then the next game starts right away
	renderFrames(s, (FINALE_END_STEP - FINALE_START_STEP + 1) * LOW_STEP);
	s.startMusic();
	EXPECT_FALSE(allZero(renderFrames(s, 8 * LOW_STEP)));
}

TEST(Synth, BlipsDecay) {
	for (Blip b : {Blip::Eat, Blip::GameOver, Blip::MenuMove, Blip::MenuSelect, Blip::Win}) {
		Synth s(RATE);
		s.trigger(b);
		std::vector<float> out = renderFrames(s, RATE / 2);
		EXPECT_FALSE(allZero(out, 0, 100)) << "blip " << static_cast<int>(b);
		EXPECT_TRUE(allZero(out, static_cast<size_t>(RATE * 4 / 10))) << "blip " << static_cast<int>(b);
	}
}

TEST(Synth, MenuSelectIsHigherThanMenuMove) {
	Synth move(RATE);
	move.trigger(Blip::MenuMove);
	Synth select(RATE);
	select.trigger(Blip::MenuSelect);
	EXPECT_GT(signChanges(renderFrames(select, 1500), 1500), signChanges(renderFrames(move, 1500), 1500));
}

TEST(Synth, ConcurrentControlIsSafe) {
	Synth s(RATE);
	std::atomic<bool> done{false};
	std::thread control([&] {
		for (int i = 0; i < 10000; ++i) {
			s.startMusic();
			s.trigger(Blip::Eat);
			s.stopMusic();
		}
		done = true;
	});
	std::vector<float> buf(256);
	while (!done) {
		s.render(buf.data(), static_cast<int>(buf.size()));
		for (float v : buf) {
			ASSERT_LE(std::fabs(v), 1.0f);
		}
	}
	control.join();
}
