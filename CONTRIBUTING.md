# Contributing

## Build and test

```
make test           # build (Release) and run the unit and e2e tests
make run            # build and play
make test-docker    # run the whole suite inside the Docker build
```

Binaries land in `build/bin/`. New headers go under `include/snake_game/<module>/` with the implementation in `src/<module>/` and a line in the `snake_core` source list in `CMakeLists.txt`.

## Documentation Conventions

### C++

Every file starts with a header comment, and every function, class and struct gets a block comment. Indent with tabs. The comment goes on the declaration in the header; the definition in the `.cpp` needs none unless it is a helper the header does not declare.

```cpp
/**
 * file_name.hpp
 * Author: Miguel Mochizuki Silva
 * Description: One line on what the file holds
 */

/**
 * What the function does, in a sentence
 *
 * Parameters:
 * const GameState& state: Current game
 * Mode mode: What each value means
 *
 * Returns int: What the result is
 */
int act(const GameState& state, Mode mode);

/**
 * What the type is for
 *
 * Attributes:
 * int side: Board side in cells
 */
```

Trivial getters need no block. Comments inside functions say why, not what.

### Python

Used by the end-to-end tests in `tests/e2e/`. Every file starts with a docstring header, and every function and class gets a docstring in the same `Parameters:` / `Returns` layout as the C++ blocks. Indent with tabs.

```python
"""
file_name.py
Author: Miguel Mochizuki Silva
Description: One line on what the file holds
"""


def choose(head, d, food):
	"""
	Greedy step toward the food that never reverses

	Parameters:
	tuple head: Head (x, y)
	tuple d: Current direction (dx, dy)
	tuple food: Food (x, y)

	Returns tuple: Next direction (dx, dy)
	"""
```
