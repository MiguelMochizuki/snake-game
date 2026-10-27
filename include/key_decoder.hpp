/**
 * key_decoder.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Turns raw terminal bytes into key events
 */
#ifndef KEY_DECODER_HPP
#define KEY_DECODER_HPP

#include <string>
#include <vector>

#include "input_handler.hpp"

/**
 * Decodes POSIX escape sequences and Windows key pairs into Key events
 *
 * Attributes:
 * State state_: Position in the escape sequence being read
 * bool hasParams_: true if the CSI sequence being read has parameter bytes
 * bool alt_: true if the sequence being read started with ESC ESC (Alt-modified)
 */
class KeyDecoder {
private:
	enum class State { Ground, Esc, Csi, Ss3 };

	State state_ = State::Ground;
	bool hasParams_ = false;
	bool alt_ = false;

	/**
	 * Decodes a byte that is not part of an escape sequence
	 *
	 * Parameters:
	 * int c: The byte, 0..255
	 *
	 * Returns Key: CtrlC, a character, or Unknown
	 */
	static Key decodeSingle(int c);

	/**
	 * Maps the final byte of an arrow sequence
	 *
	 * Parameters:
	 * char c: The final byte
	 *
	 * Returns Key: Up, Down, Right or Left for A to D, else Unknown
	 */
	static Key decodeArrow(char c);

public:
	/**
	 * Decodes bytes read from a POSIX terminal. Partial sequences are kept for the next call
	 *
	 * Parameters:
	 * const std::string& bytes: The bytes read
	 *
	 * Returns std::vector<Key>: The keys completed by these bytes
	 */
	std::vector<Key> feed(const std::string& bytes);

	/**
	 * Called when a poll returned no bytes. Resolves a pending lone ESC
	 *
	 * Returns std::vector<Key>: Unknown if an ESC was pending, else empty
	 */
	std::vector<Key> idle();

	/**
	 * Drops any pending partial sequence
	 */
	void reset();

	/**
	 * Decodes a Windows _getch result
	 *
	 * Parameters:
	 * int first: First byte, 0..255
	 * int second: Second byte, used only when first is a prefix
	 *
	 * Returns Key: The decoded key
	 */
	static Key decodeWindows(int first, int second);

	/**
	 * Tells whether a byte starts a Windows two-byte key
	 *
	 * Parameters:
	 * int c: The byte, 0..255
	 *
	 * Returns bool: true if c is 0x00 or 0xE0
	 */
	static bool isWindowsPrefix(int c);
};

#endif
