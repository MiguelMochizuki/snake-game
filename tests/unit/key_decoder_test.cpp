/**
 * key_decoder_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for KeyDecoder
 */
#include <gtest/gtest.h>

#include "key_decoder.hpp"

using Keys = std::vector<Key>;

static Key sk(KeyKind k) { return specialKey(k); }

TEST(KeyDecoder, PrintableChars) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("wasdq"), (Keys{charKey('w'), charKey('a'), charKey('s'), charKey('d'), charKey('q')}));
}

TEST(KeyDecoder, CtrlCIsCtrlC) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x03"), (Keys{sk(KeyKind::CtrlC)}));
}

TEST(KeyDecoder, ArrowsCsi) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b[A\x1b[B\x1b[C\x1b[D"),
		(Keys{sk(KeyKind::Up), sk(KeyKind::Down), sk(KeyKind::Right), sk(KeyKind::Left)}));
}

TEST(KeyDecoder, ArrowsSs3) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1bOA\x1bOB\x1bOC\x1bOD"),
		(Keys{sk(KeyKind::Up), sk(KeyKind::Down), sk(KeyKind::Right), sk(KeyKind::Left)}));
}

TEST(KeyDecoder, LeftArrowNeverSteersRight) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b[D"), (Keys{sk(KeyKind::Left)}));
}

TEST(KeyDecoder, SplitArrowAcrossFeeds) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b"), Keys{});
	EXPECT_EQ(d.feed("[D"), (Keys{sk(KeyKind::Left)}));
	EXPECT_EQ(d.feed("\x1b["), Keys{});
	EXPECT_EQ(d.feed("D"), (Keys{sk(KeyKind::Left)}));
}

TEST(KeyDecoder, LoneEscResolvedByIdle) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b"), Keys{});
	EXPECT_EQ(d.idle(), (Keys{sk(KeyKind::Unknown)}));
	EXPECT_EQ(d.idle(), Keys{});
}

TEST(KeyDecoder, IdleLeavesIncompleteCsiPending) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b["), Keys{});
	EXPECT_EQ(d.idle(), Keys{});
	EXPECT_EQ(d.feed("D"), (Keys{sk(KeyKind::Left)}));
}

TEST(KeyDecoder, LoneEscThenKeyNextFeed) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b"), Keys{});
	EXPECT_EQ(d.feed("q"), (Keys{sk(KeyKind::Unknown), charKey('q')}));
}

TEST(KeyDecoder, AltKeyIsOneUnknown) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1bq"), (Keys{sk(KeyKind::Unknown)}));
	EXPECT_EQ(d.feed("\x1b" "d"), (Keys{sk(KeyKind::Unknown)}));
}

TEST(KeyDecoder, ModifiedArrowUnknown) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b[1;5A"), (Keys{sk(KeyKind::Unknown)}));
}

TEST(KeyDecoder, FunctionAndNavKeysUnknown) {
	for (const char* s : {"\x1bOP", "\x1bOQ", "\x1bOR", "\x1bOS", "\x1b[15~", "\x1b[21~", "\x1b[24~",
			"\x1b[H", "\x1b[F", "\x1b[2~", "\x1b[3~", "\x1b[5~", "\x1b[6~"}) {
		KeyDecoder d;
		EXPECT_EQ(d.feed(s), (Keys{sk(KeyKind::Unknown)})) << s;
	}
}

TEST(KeyDecoder, ControlBytesUnknown) {
	for (const char* s : {"\x1a", "\x1c", "\x13", "\x11", "\x7f"}) {
		KeyDecoder d;
		EXPECT_EQ(d.feed(s), (Keys{sk(KeyKind::Unknown)}));
	}
}

TEST(KeyDecoder, Utf8Unknown) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\xc3\xa9"), (Keys{sk(KeyKind::Unknown), sk(KeyKind::Unknown)}));
}

TEST(KeyDecoder, ResetDropsPending) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b["), Keys{});
	d.reset();
	EXPECT_EQ(d.feed("D"), (Keys{charKey('D')}));
}

TEST(KeyDecoder, WindowsArrows) {
	for (int p : {0xE0, 0x00}) {
		EXPECT_TRUE(KeyDecoder::isWindowsPrefix(p));
		EXPECT_EQ(KeyDecoder::decodeWindows(p, 0x48), sk(KeyKind::Up));
		EXPECT_EQ(KeyDecoder::decodeWindows(p, 0x50), sk(KeyKind::Down));
		EXPECT_EQ(KeyDecoder::decodeWindows(p, 0x4B), sk(KeyKind::Left));
		EXPECT_EQ(KeyDecoder::decodeWindows(p, 0x4D), sk(KeyKind::Right));
	}
	EXPECT_FALSE(KeyDecoder::isWindowsPrefix('w'));
}

TEST(KeyDecoder, WindowsExtendedKeysUnknown) {
	EXPECT_EQ(KeyDecoder::decodeWindows(0xE0, 0x51), sk(KeyKind::Unknown));
	EXPECT_EQ(KeyDecoder::decodeWindows(0xE0, 0x53), sk(KeyKind::Unknown));
	EXPECT_EQ(KeyDecoder::decodeWindows(0x00, 0x41), sk(KeyKind::Unknown));
	EXPECT_EQ(KeyDecoder::decodeWindows(0x00, 0x44), sk(KeyKind::Unknown));
	EXPECT_EQ(KeyDecoder::decodeWindows(0xE0, 0x47), sk(KeyKind::Unknown));
}

TEST(KeyDecoder, WindowsSingleBytes) {
	EXPECT_EQ(KeyDecoder::decodeWindows(3, 0), sk(KeyKind::CtrlC));
	EXPECT_EQ(KeyDecoder::decodeWindows('w', 0), charKey('w'));
	EXPECT_EQ(KeyDecoder::decodeWindows(0x1a, 0), sk(KeyKind::Unknown));
}

TEST(KeyDecoder, AbortedCsiReprocessesAbortingByte) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b[\x1b[D"), (Keys{sk(KeyKind::Unknown), sk(KeyKind::Left)}));
	EXPECT_EQ(d.feed("\x1b[\x03"), (Keys{sk(KeyKind::Unknown), sk(KeyKind::CtrlC)}));
}

TEST(KeyDecoder, AbortedSs3ReprocessesAbortingByte) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1bO\x1b[A"), (Keys{sk(KeyKind::Unknown), sk(KeyKind::Up)}));
}

TEST(KeyDecoder, EscEscIsAltModifiedSingleUnknown) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b\x1b[D"), (Keys{sk(KeyKind::Unknown)}));
	EXPECT_EQ(d.feed("\x1b\x1bOA"), (Keys{sk(KeyKind::Unknown)}));
	EXPECT_EQ(d.feed("\x1b\x1bq"), (Keys{sk(KeyKind::Unknown)}));
}

TEST(KeyDecoder, EscEscSplitAcrossFeeds) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1b\x1b"), Keys{});
	EXPECT_EQ(d.feed("[D"), (Keys{sk(KeyKind::Unknown)}));
	EXPECT_EQ(d.feed("\x1b\x1b"), Keys{});
	EXPECT_EQ(d.idle(), (Keys{sk(KeyKind::Unknown)}));
}

TEST(KeyDecoder, IdleLeavesIncompleteSs3Pending) {
	KeyDecoder d;
	EXPECT_EQ(d.feed("\x1bO"), Keys{});
	EXPECT_EQ(d.idle(), Keys{});
	EXPECT_EQ(d.feed("A"), (Keys{sk(KeyKind::Up)}));
}
