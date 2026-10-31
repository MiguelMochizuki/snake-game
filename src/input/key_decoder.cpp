/**
 * key_decoder.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Implementation of KeyDecoder
 */
#include "snake_game/input/key_decoder.hpp"

Key KeyDecoder::decodeSingle(int c) {
	if (c == 0x03) return specialKey(KeyKind::CtrlC);
	if (c == 0x0D || c == 0x0A) return specialKey(KeyKind::Enter);
	if (c >= 0x20 && c <= 0x7E) return charKey(static_cast<char>(c));
	return specialKey(KeyKind::Unknown);
}

Key KeyDecoder::decodeArrow(char c) {
	switch (c) {
		case 'A': return specialKey(KeyKind::Up);
		case 'B': return specialKey(KeyKind::Down);
		case 'C': return specialKey(KeyKind::Right);
		case 'D': return specialKey(KeyKind::Left);
		default: return specialKey(KeyKind::Unknown);
	}
}

std::vector<Key> KeyDecoder::feed(const std::string& bytes) {
	std::vector<Key> out;
	// Emits k, or Unknown if the sequence was Alt-modified (ESC ESC ...)
	auto emit = [&](Key k) {
		out.push_back(alt_ ? specialKey(KeyKind::Unknown) : k);
		alt_ = false;
	};
	// Across feeds a pending ESC followed by a plain byte is a lone ESC, so the byte is
	// decoded from scratch. Within one feed (case Esc below) ESC + byte is Alt+byte.
	if (state_ == State::Esc && !alt_ && !bytes.empty()
			&& bytes[0] != '[' && bytes[0] != 'O' && bytes[0] != '\x1b') {
		emit(specialKey(KeyKind::Unknown));
		state_ = State::Ground;
	}
	for (size_t i = 0; i < bytes.size(); i++) {
		const char ch = bytes[i];
		const int c = static_cast<unsigned char>(ch);
		switch (state_) {
			case State::Ground:
				if (c == 0x1B) state_ = State::Esc;
				else out.push_back(decodeSingle(c));
				break;
			case State::Esc:
				if (c == 0x1B) {
					alt_ = true;
				} else if (ch == '[') {
					state_ = State::Csi;
					hasParams_ = false;
				} else if (ch == 'O') {
					state_ = State::Ss3;
					hasParams_ = false;
				} else {
					emit(specialKey(KeyKind::Unknown));
					state_ = State::Ground;
				}
				break;
			case State::Csi:
				if (c >= 0x20 && c <= 0x3F) {
					hasParams_ = true;
				} else if (c >= 0x40 && c <= 0x7E) {
					emit(hasParams_ ? specialKey(KeyKind::Unknown) : decodeArrow(ch));
					state_ = State::Ground;
				} else {
					// Invalid byte: abort the sequence and decode this byte from Ground
					emit(specialKey(KeyKind::Unknown));
					state_ = State::Ground;
					i--;
				}
				break;
			case State::Ss3:
				if (c >= 0x20 && c <= 0x3F) {
					hasParams_ = true;
				} else if (c >= 0x40 && c <= 0x7E) {
					emit(hasParams_ ? specialKey(KeyKind::Unknown) : decodeArrow(ch));
					state_ = State::Ground;
				} else {
					emit(specialKey(KeyKind::Unknown));
					state_ = State::Ground;
					i--;
				}
				break;
		}
	}
	return out;
}

std::vector<Key> KeyDecoder::idle() {
	std::vector<Key> out;
	if (state_ == State::Esc) {
		out.push_back(specialKey(KeyKind::Unknown));
		state_ = State::Ground;
		alt_ = false;
	}
	return out;
}

void KeyDecoder::reset() {
	alt_ = false;
	state_ = State::Ground;
	hasParams_ = false;
}

bool KeyDecoder::isWindowsPrefix(int c) {
	return c == 0x00 || c == 0xE0;
}

Key KeyDecoder::decodeWindows(int first, int second) {
	if (!isWindowsPrefix(first)) return decodeSingle(first);
	switch (second) {
		case 0x48: return specialKey(KeyKind::Up);
		case 0x50: return specialKey(KeyKind::Down);
		case 0x4B: return specialKey(KeyKind::Left);
		case 0x4D: return specialKey(KeyKind::Right);
		default: return specialKey(KeyKind::Unknown);
	}
}
