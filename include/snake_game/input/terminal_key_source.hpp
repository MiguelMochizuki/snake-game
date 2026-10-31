/**
 * terminal_key_source.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Key source reading the raw terminal
 */
#ifndef TERMINAL_KEY_SOURCE_HPP
#define TERMINAL_KEY_SOURCE_HPP

#include <deque>
#include <optional>
#include <vector>

#ifndef _WIN32
	#include <termios.h>
#endif

#include "snake_game/input/key_decoder.hpp"
#include "snake_game/input/key_source.hpp"

/**
 * Reads keys from stdin. The constructor puts the terminal in raw mode and the destructor restores it
 *
 * Attributes:
 * KeyDecoder decoder_: Decoder for escape sequences, keeps state across reads
 * std::deque<Key> queue_: Decoded keys not yet returned
 * bool termiosSaved_: (POSIX) true if the terminal settings were saved
 * struct termios originalTerm_: (POSIX) Terminal settings to restore
 * int originalFlags_: (POSIX) stdin file status flags to restore, -1 if none
 * bool modeSaved_: (Windows) true if the console mode was saved
 * unsigned long originalMode_: (Windows) Console input mode to restore
 */
class TerminalKeySource : public IKeySource {
private:
	KeyDecoder decoder_;
	std::deque<Key> queue_;
#ifdef _WIN32
	bool modeSaved_ = false;
	unsigned long originalMode_ = 0;
#else
	bool termiosSaved_ = false;
	struct termios originalTerm_;
	int originalFlags_ = -1;
#endif

public:
	/**
	 * Constructor for the TerminalKeySource class. Enters raw mode
	 */
	TerminalKeySource();

	/**
	 * Destructor for the TerminalKeySource class. Restores the terminal
	 */
	~TerminalKeySource() override;

	TerminalKeySource(const TerminalKeySource&) = delete;
	TerminalKeySource& operator=(const TerminalKeySource&) = delete;

	/**
	 * Reads every key available now, without blocking
	 *
	 * Returns std::vector<Key>: The keys, oldest first; empty if none
	 */
	std::vector<Key> poll() override;

	/**
	 * Blocks until a key is available
	 *
	 * Returns std::optional<Key>: The key, or nullopt on EOF or a hard error
	 */
	std::optional<Key> waitKey() override;

	/**
	 * Discards pending input, the key queue and any partial escape sequence
	 */
	void flush() override;
};

#endif
