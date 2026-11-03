/**
 * synth_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the chord sequencer and blip synth
 */
#include <gtest/gtest.h>

#include <atomic>
#include <cmath>
#include <thread>
#include <vector>

#include "snake_game/audio/synth.hpp"

namespace {

const int RATE = 44100;
const int LOOP_FRAMES = 302400;  // 64 sixteenths of 4725 samples at 140 BPM

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

TEST(Synth, StepFrequenciesFollowProgression) {
	EXPECT_NEAR(stepFrequency(0), 261.63, 0.01);
	EXPECT_NEAR(stepFrequency(1), 329.63, 0.01);
	EXPECT_NEAR(stepFrequency(2), 392.00, 0.01);
	EXPECT_NEAR(stepFrequency(3), 329.63, 0.01);
	EXPECT_NEAR(stepFrequency(16), 220.00, 0.01);
	EXPECT_NEAR(stepFrequency(32), 174.61, 0.01);
	EXPECT_NEAR(stepFrequency(48), 196.00, 0.01);
	EXPECT_DOUBLE_EQ(stepFrequency(64), stepFrequency(0));
}

TEST(Synth, LoopIsSevenSeconds) {
	Synth s(RATE);
	s.startMusic();
	std::vector<float> first = renderFrames(s, LOOP_FRAMES);
	std::vector<float> second = renderFrames(s, LOOP_FRAMES);
	EXPECT_FALSE(allZero(first));
	EXPECT_EQ(first, second);
}

TEST(Synth, SilentWhenStopped) {
	Synth s(RATE);
	EXPECT_TRUE(allZero(renderFrames(s, 5000)));
	s.startMusic();
	EXPECT_FALSE(allZero(renderFrames(s, 5000)));
	s.stopMusic();
	EXPECT_TRUE(allZero(renderFrames(s, 5000)));
}

TEST(Synth, BlipsDecay) {
	for (Blip b : {Blip::Eat, Blip::GameOver, Blip::MenuMove, Blip::MenuSelect}) {
		Synth s(RATE);
		s.trigger(b);
		std::vector<float> out = renderFrames(s, RATE / 2);
		EXPECT_FALSE(allZero(out, 0, 100)) << "blip " << static_cast<int>(b);
		EXPECT_TRUE(allZero(out, static_cast<size_t>(RATE * 3 / 10))) << "blip " << static_cast<int>(b);
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
