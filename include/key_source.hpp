/**
 * key_source.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Interface for a source of decoded key presses
 */
#ifndef KEY_SOURCE_HPP
#define KEY_SOURCE_HPP

#include <optional>
#include <vector>

#include "input_handler.hpp"

/**
 * Source of decoded key presses
 */
class IKeySource {
public:
	/**
	 * Destructor for the IKeySource interface
	 */
	virtual ~IKeySource() = default;

	/**
	 * Reads every key available now, without blocking
	 *
	 * Returns std::vector<Key>: The keys, oldest first; empty if none
	 */
	virtual std::vector<Key> poll() = 0;

	/**
	 * Blocks until a key is available
	 *
	 * Returns std::optional<Key>: The key, or nullopt on EOF or a hard error
	 */
	virtual std::optional<Key> waitKey() = 0;

	/**
	 * Discards pending input and any partial escape sequence
	 */
	virtual void flush() = 0;
};

#endif
