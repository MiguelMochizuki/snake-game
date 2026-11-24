/**
 * synth_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the chord sequencer and blip synth
 */
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include "snake_game/audio/score.hpp"
#include "snake_game/audio/synth.hpp"

namespace {

const int RATE = 44100;
const int LOW_RATE = 2000;  // keeps whole-repertoire renders small

/**
 * Number of samples of a duration
 *
 * Parameters:
 * int rate: Sample rate in Hz
 * long long ms: Duration in milliseconds
 *
 * Returns int: The samples
 */
int framesOf(int rate, long long ms) {
	return static_cast<int>(ms * rate / 1000);
}

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

// Sets (or clears, with nullptr) an environment variable on both POSIX and Windows
void setVar(const char* name, const char* value) {
#ifdef _WIN32
	_putenv_s(name, value != nullptr ? value : "");
#elif defined(__unix__) || defined(__APPLE__)
	if (value != nullptr) {
		setenv(name, value, 1);
	} else {
		unsetenv(name);
	}
#endif
}

}  // namespace

TEST(Score, TheRepertoireHasEightPiecesAndTwoEasterEggsStartingWithTheAveMaria) {
	EXPECT_EQ(PIECE_COUNT, 10);
	EXPECT_STREQ(PIECES[0].title, "Ave Maria");
	int rare = 0;
	for (int i = 0; i < PIECE_COUNT; ++i) {
		EXPECT_GT(std::string(PIECES[i].title).size(), 0u);
		EXPECT_GT(std::string(PIECES[i].composer).size(), 0u);
		rare += PIECES[i].rare ? 1 : 0;
	}
	EXPECT_EQ(rare, 2);
	EXPECT_FALSE(PIECES[0].rare);
}

TEST(Score, EveryPieceIsConsistent) {
	for (int i = 0; i < PIECE_COUNT; ++i) {
		const Piece& piece = PIECES[i];
		int last = 0, end = 0, melody = 0;
		for (int n = 0; n < piece.count; ++n) {
			const ScoreNote& note = piece.notes[n];
			ASSERT_GE(note.start, last) << piece.title << " note " << n << " is out of order";
			ASSERT_GT(note.length, 0) << piece.title << " note " << n;
			ASSERT_GE(note.pitch, 21) << piece.title << " note " << n;
			ASSERT_LE(note.pitch, 108) << piece.title << " note " << n;
			ASSERT_GE(note.velocity, 1) << piece.title << " note " << n;
			ASSERT_LE(note.velocity, 127) << piece.title << " note " << n;
			last = note.start;
			end = std::max(end, note.start + note.length);
			melody += note.melody;
		}
		EXPECT_EQ(end, piece.endMs) << piece.title;
		EXPECT_GT(melody, 0) << piece.title << " has no melody";
		EXPECT_GT(piece.count, melody) << piece.title << " has no accompaniment";
		EXPECT_GE(piece.endMs, 30 * 1000) << piece.title << " is too short";
		EXPECT_LE(piece.endMs, 10 * 60 * 1000) << piece.title << " is too long";
	}
}

TEST(Score, AveMariaOpensLikeBachAndTheMelodyWaitsFourBars) {
	const Piece& piece = PIECES[0];
	// Bar 1 of the C major prelude: C E G C E G C E
	const int bar1[8] = {60, 64, 67, 72, 76, 67, 72, 76};
	int seen = 0;
	int firstMelody = -1;
	for (int n = 0; n < piece.count; ++n) {
		const ScoreNote& note = piece.notes[n];
		if (!note.melody && seen < 8) {
			EXPECT_EQ(note.pitch, bar1[seen]) << "accompaniment note " << seen;
			++seen;
		}
		if (note.melody && firstMelody < 0) {
			firstMelody = n;
		}
	}
	ASSERT_GE(firstMelody, 0);
	EXPECT_EQ(piece.notes[firstMelody].pitch, 76);  // E5, first note of the melody
	EXPECT_NEAR(piece.notes[firstMelody].start, 64 * 60000 / 66 / 4, 20);  // after four bars at 66 BPM
}

TEST(Synth, WhenAPieceEndsAfterABreathAnotherOneStartsAtRandom) {
	Synth s(LOW_RATE, 7);
	s.startMusic();
	ASSERT_EQ(s.piece(), 0);
	std::vector<float> first = renderFrames(s, framesOf(LOW_RATE, PIECES[0].endMs + REST_BETWEEN_PIECES_MS));
	EXPECT_FALSE(allZero(first));
	// The rest after the piece is silent
	EXPECT_TRUE(allZero(first, static_cast<size_t>(framesOf(LOW_RATE, PIECES[0].endMs + 200))));
	// Another piece follows at once, never the same one
	EXPECT_NE(s.piece(), 0);
	EXPECT_FALSE(allZero(renderFrames(s, framesOf(LOW_RATE, 30000))));
}

TEST(Synth, ARandomPieceNeverRepeatsTheLastOneAndEveryPieceCanCome) {
	Synth s(LOW_RATE, 1);
	std::vector<bool> seen(PIECE_COUNT, false);
	int last = s.piece();
	for (int i = 0; i < 3000; ++i) {
		s.startRandomPiece();
		renderFrames(s, 1);
		EXPECT_NE(s.piece(), last);
		last = s.piece();
		seen[last] = true;
	}
	EXPECT_EQ(std::count(seen.begin(), seen.end(), true), PIECE_COUNT);
}

TEST(Synth, SnakePieceForcesAPieceByNumberOrByTitle) {
	setVar("SNAKE_PIECE", "marseillaise");
	Synth byTitle(LOW_RATE, 1);
	setVar("SNAKE_PIECE", "3");
	Synth byNumber(LOW_RATE, 1);
	setVar("SNAKE_PIECE", nullptr);
	EXPECT_STREQ(PIECES[byTitle.piece()].title, "La Marseillaise");
	EXPECT_EQ(byNumber.piece(), 2);
	for (int i = 0; i < 5; ++i) {
		byTitle.startRandomPiece();
		renderFrames(byTitle, 1);
		EXPECT_STREQ(PIECES[byTitle.piece()].title, "La Marseillaise");
	}
}

TEST(Synth, EachEasterEggComesAboutOnceInAHundredGames) {
	Synth s(LOW_RATE, 3);
	std::vector<int> count(PIECE_COUNT, 0);
	const int games = 20000;
	for (int i = 0; i < games; ++i) {
		s.startRandomPiece();
		renderFrames(s, 1);
		++count[s.piece()];
	}
	for (int i = 0; i < PIECE_COUNT; ++i) {
		const double share = static_cast<double>(count[i]) / games;
		if (PIECES[i].rare) {
			EXPECT_NEAR(share, 0.01, 0.005) << PIECES[i].title;
		} else {
			EXPECT_GT(share, 0.08) << PIECES[i].title;
		}
	}
}

TEST(Synth, TheSameSeedChoosesTheSamePiecesAndANewPieceStartsFromItsBeginning) {
	Synth a(LOW_RATE, 5), b(LOW_RATE, 5);
	for (int i = 0; i < 20; ++i) {
		a.startRandomPiece();
		b.startRandomPiece();
		EXPECT_EQ(renderFrames(a, 500), renderFrames(b, 500));
		EXPECT_EQ(a.piece(), b.piece());
		renderFrames(a, 20000);  // far into the piece: the next choice must still start over
		renderFrames(b, 20000);
	}
	Synth fresh(LOW_RATE, 5);
	fresh.startRandomPiece();
	Synth late(LOW_RATE, 5);
	late.startRandomPiece();
	renderFrames(late, 30000);
	late.stopMusic();
	renderFrames(late, 1);
	// Choosing again after having played moves on to a different piece and starts it from its beginning
	late.startRandomPiece();
	renderFrames(late, 1);
	Synth reference(LOW_RATE, 5);
	reference.startRandomPiece();
	renderFrames(reference, 1);
	reference.startRandomPiece();
	renderFrames(reference, 1);
	EXPECT_EQ(late.piece(), reference.piece());
}

TEST(Synth, StoppingKeepsThePlaceAndStartingGoesOnFromIt) {
	Synth continuous(LOW_RATE), paused(LOW_RATE);
	continuous.startMusic();
	std::vector<float> whole = renderFrames(continuous, 60000);

	paused.startMusic();
	std::vector<float> before = renderFrames(paused, 30000);
	paused.stopMusic();
	EXPECT_TRUE(allZero(renderFrames(paused, 5000)));
	paused.startMusic();
	std::vector<float> after = renderFrames(paused, 30000);
	EXPECT_EQ(before, std::vector<float>(whole.begin(), whole.begin() + 30000));
	EXPECT_EQ(after, std::vector<float>(whole.begin() + 30000, whole.end()));
}

TEST(Synth, MelodyAddsToTheAccompaniment) {
	// The first four bars of the Ave Maria are accompaniment only; the melody enters after them and the sound gets louder
	Synth s(LOW_RATE);
	s.startMusic();
	std::vector<float> out = renderFrames(s, framesOf(LOW_RATE, 29000));
	double intro = 0, withMelody = 0;
	for (int i = 0; i < framesOf(LOW_RATE, 14000); ++i) {
		intro += std::fabs(out[i]);
		withMelody += std::fabs(out[i + framesOf(LOW_RATE, 15000)]);
	}
	EXPECT_GT(withMelody, intro * 1.1);
}

TEST(Synth, MusicStaysQuietAndFinite) {
	// The whole repertoire, so the loudest chord of any piece is covered
	Synth s(LOW_RATE);
	s.startMusic();
	long long total = 0;
	for (int i = 0; i < PIECE_COUNT; ++i) {
		total += framesOf(LOW_RATE, PIECES[i].endMs + REST_BETWEEN_PIECES_MS);
	}
	float peak = 0.0f;
	for (float v : renderFrames(s, static_cast<int>(total))) {
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

TEST(Synth, TheLastMusicCommandWinsWhenSeveralArriveBetweenCallbacks) {
	const int step = framesOf(LOW_RATE, 1000);
	Synth stopped(LOW_RATE);
	stopped.startMusic();
	renderFrames(stopped, 5 * step);
	stopped.startMusic();
	stopped.stopMusic();  // quit right after a restart, before the audio thread ran
	EXPECT_TRUE(allZero(renderFrames(stopped, 4 * step)));

	Synth again(LOW_RATE), fresh(LOW_RATE);
	again.startMusic();
	renderFrames(again, 5 * step);
	again.stopMusic();
	again.startMusic();  // and the other way round: the music goes on, with no gap
	fresh.startMusic();
	std::vector<float> whole = renderFrames(fresh, 9 * step);
	EXPECT_EQ(renderFrames(again, 4 * step), std::vector<float>(whole.begin() + 5 * step, whole.end()));
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
