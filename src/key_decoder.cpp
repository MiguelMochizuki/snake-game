/**
 * key_decoder.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Implementation of KeyDecoder
 */
#include "key_decoder.hpp"

Key KeyDecoder::decodeSingle(int c) {
	if (c == 0x03) return specialKey(KeyKind::CtrlC);
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
	size_t i = 0;
	// A pending ESC from an earlier feed followed by a non-sequence byte resolves alone
	if (state_ == State::Esc && !bytes.empty() && bytes[0] != '[' && bytes[0] != 'O') {
		out.push_back(specialKey(KeyKind::Unknown));
		state_ = State::Ground;
	}
	for (; i < bytes.size(); i++) {
		const char ch = bytes[i];
		const int c = static_cast<unsigned char>(ch);
		switch (state_) {
			case State::Ground:
				if (c == 0x1B) state_ = State::Esc;
				else out.push_back(decodeSingle(c));
				break;
			case State::Esc:
				if (ch == '[') {
					state_ = State::Csi;
					hasParams_ = false;
				} else if (ch == 'O') {
					state_ = State::Ss3;
				} else {
					out.push_back(specialKey(KeyKind::Unknown));
					state_ = State::Ground;
				}
				break;
			case State::Csi:
				if (c >= 0x20 && c <= 0x3F) {
					hasParams_ = true;
				} else {
					// Final byte, or an invalid byte that aborts the sequence
					out.push_back(c >= 0x40 && c <= 0x7E && !hasParams_
						? decodeArrow(ch) : specialKey(KeyKind::Unknown));
					state_ = State::Ground;
				}
				break;
			case State::Ss3:
				out.push_back(decodeArrow(ch));
				state_ = State::Ground;
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
	}
	return out;
}

void KeyDecoder::reset() {
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
