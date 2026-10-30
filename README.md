# Snake Game

The classic Snake game for the terminal, written in C++. It saves your scores between sessions and also runs in Docker.

## Features

- Start menu with Start, Quit and three levels: Easy, Normal and Hard
- Arrow keys or WASD
- Best score, games played and average score, saved between sessions
- Colored board (cyan walls, green snake with a bright head, red food, yellow numbers)
- Linux, macOS and Windows binaries, plus a Docker image

## Download

Binaries for each release are on the [Releases](https://github.com/miguelmochizuki/snake-game/releases) page. The latest is [v1.4.0](https://github.com/miguelmochizuki/snake-game/releases/tag/v1.4.0), which adds the start menu and difficulty levels.

| Platform | Architecture | Binary |
|----------|-------------|--------|
| Linux | x86-64 | `snake-linux-x86_64` |
| Linux | ARM64 | `snake-linux-arm64` |
| Windows | x86-64 | `snake-windows-x86_64.exe` |
| Windows | ARM64 | `snake-windows-arm64.exe` |
| macOS | Universal (x86_64 + ARM64) | `snake-macos-universal` |

On Linux and macOS, make the file executable and run it:

```bash
chmod +x snake-linux-x86_64
./snake-linux-x86_64
```

On Windows, double-click the `.exe` or run it from a terminal that supports ANSI escape codes, such as Windows Terminal.

## Build from source

You need a C++17 compiler (g++ 7+, clang 5+ or MSVC 2017+), CMake 3.14+ and Make or Ninja.

```bash
make run-local       # build and play
make local           # build only: build/bin/snake
make rebuild-local   # clean build
make clean-local     # remove build files
```

Without Make:

```bash
mkdir build && cd build
cmake ..
make
./bin/snake
```

For a different build type, pass `-DCMAKE_BUILD_TYPE=Debug` (or `Release`, the default, or `RelWithDebInfo`) to `cmake`. All targets compile with `-Wall -Wextra -Wpedantic` (`/W4` on MSVC).

On Ubuntu or Debian, `sudo apt-get install build-essential cmake` installs the dependencies. On macOS, use `brew install cmake`. On Windows with MinGW, run `cmake -G "MinGW Makefiles" ..`, then `mingw32-make`.

## Docker

```bash
docker pull miguelmochizuki/snake-game
docker run --rm -it -v $(pwd)/data:/app/data miguelmochizuki/snake-game
```

The image is published for linux/amd64 and linux/arm64. Without the `-v` option, scores are lost when the container exits.

To build the image yourself (Docker 20.10 or newer):

```bash
make run-docker      # build the image and play with a persistent score volume
make docker          # build only
make rebuild-docker  # clean build
make clean-docker    # remove the image and container
```

The image builds in two stages (Alpine with build tools, then a minimal Alpine runtime) and runs as the non-root user `snakeuser`.

## How to play

The game opens on a menu. Pick a level, then start.

| Key | In the menu | In the game |
|-----|-------------|-------------|
| W / Up arrow | Move selection up | Move up |
| S / Down arrow | Move selection down | Move down |
| A / Left arrow | Previous level (on the Level row) | Move left |
| D / Right arrow | Next level (on the Level row) | Move right |
| Enter | Choose the selected row | Nothing |
| Q / Ctrl+C | Quit | End the game |

The levels set the time per step: Easy 300 ms, Normal 200 ms, Hard 100 ms. The menu also shows your best score, games played and average over the last 10 games. Any other key does nothing.

After a game ends, you see your final score and the same statistics. Press **R** to play again (straight away, without the menu) or **Q** to quit.

Set `NO_COLOR` to any non-empty value to turn colors off, for example `NO_COLOR=1 ./snake`. An empty value keeps them on, as [no-color.org](https://no-color.org) describes.

### Scores

Scores are saved in the `data/` directory, which the game creates:

- `best_score.txt` holds your highest score.
- `score_history.txt` holds the score of every game.

## Code layout

- `src/` and `include/` hold the game. `GameEngine` runs the loop, `GameState` holds the snake and food, and `TerminalRenderer` draws the board.
- Input goes through a key source and a set of handlers (WASD, arrows and commands), so the engine never reads the terminal directly.
- The menu is split the same way: `MenuModel` holds its state, `IMenuView` draws it (`TerminalMenuView` does so in ASCII), and `runMenu` connects them to the keys. A different interface, such as a GUI, would need only another `IMenuView`.
- `tests/` holds the unit, golden and end-to-end tests.

## Tests

The suite has three parts, all run by CTest:

- **Unit** (`tests/unit/`): GoogleTest tests for the game classes, scores, input handling, the renderer and the menu. CMake downloads GoogleTest, so the first configure needs network access.
- **Golden** (`tests/golden/`): saved renderer and menu output that the tests compare against.
- **End-to-end** (`tests/e2e/`): Python 3 scripts that run `snake` in a pseudo-terminal and press keys. Each `test_*.py` registers itself as `e2e_<name>`. They run on Linux only (x86-64 and ARM64, natively and in Docker), while the unit tests also run on macOS and Windows.

```bash
make test-local                      # build, then run everything
make test-docker                     # run the suite inside the Docker build
ctest --test-dir build -L unit       # unit tests only
ctest --test-dir build -L e2e        # end-to-end tests only
ctest --test-dir build -R <name>     # one test, e.g. -R e2e_wall_death
```

`make docker` and `make run-docker` build through the test stage, so a failing test stops the image from being built.

If you change the renderer or menu on purpose, regenerate the golden files and check the diff before committing:

```bash
SNAKE_UPDATE_GOLDEN=1 ctest --test-dir build -R "TerminalRenderer|TerminalMenuView"
git diff tests/golden/
```

### Environment variables

Normal play needs none of these. The tests use them to make the game predictable.

- `SNAKE_FRAME_MS` (0 to 10000) sets the time per step. When valid, it overrides the level picked in the menu. Without the menu it defaults to 200.
- `SNAKE_SEED` (unsigned integer) sets the random seed for food placement.
- `SNAKE_SKIP_MENU` (any non-empty value) starts the game without the menu. The end-to-end harness sets it to `1` by default.
- `SNAKE_E2E_TIMEOUT_SCALE` (integer, default 1) multiplies the end-to-end timeouts and the step time. Raise it on slow machines. The Docker build uses 10.

## CI and releases

One GitHub Actions workflow handles both. On every push to `main` and every pull request, it builds and tests Linux x86-64, Linux ARM64 (on a native ARM64 runner), Windows x86-64, Windows ARM64 and macOS Universal. The Docker image runs the whole suite as part of its build, for amd64 and arm64 (arm64 under emulation).

To publish a release, push a tag that starts with `v`:

```bash
git tag -a v1.4.0 -m "Release v1.4.0"
git push origin v1.4.0
```

The workflow then creates a GitHub release with all the binaries and publishes the Docker image to Docker Hub. A failing platform job blocks the release. The image builds at the same time as the binaries.

### Version history

| Version | What changed |
|---------|--------------|
| [v1.4.0](https://github.com/miguelmochizuki/snake-game/releases/tag/v1.4.0) | Start menu with Easy, Normal and Hard levels and score statistics, behind an `IMenuView` interface; Enter key; `SNAKE_SKIP_MENU` test hook |
| [v1.3.0](https://github.com/miguelmochizuki/snake-game/releases/tag/v1.3.0) | Arrow keys; strict key handling (escape sequences, Windows extended keys, Ctrl+C/Z/S); average over the last 10 games |
| [v1.2.0](https://github.com/miguelmochizuki/snake-game/releases/tag/v1.2.0) | Colors with `NO_COLOR` support; exit on end of input at game over; the Makefile tracks headers; CTest suite (unit, golden, Linux end-to-end) on every platform and in Docker |
| [v1.1.0](https://github.com/miguelmochizuki/snake-game/releases/tag/v1.1.0) | Smaller memory use (flattened occupancy bitmask); buffered terminal output for smoother drawing |
| [v1.0.0](https://github.com/miguelmochizuki/snake-game/releases/tag/v1.0.0) | First release: cross-platform builds, Docker, saved scores |

## Troubleshooting

**Input does not work.** The terminal must support raw input mode. On Windows, use one that understands ANSI escape codes, such as Windows Terminal or ConEmu.

**Docker permission errors.** Add your user to the `docker` group, then log out and back in:

```bash
sudo usermod -aG docker $USER
```

**Scores are not saved in Docker.** Mount `data/` as a volume (`-v $(pwd)/data:/app/data`) and check its permissions with `chmod 755 data/`.

## Contributing

Fork the repository, make your changes on a branch, run `make test-local` (and try the game with `make run-docker`), then open a pull request.

Ideas not done yet: a leaderboard, sound (terminal beep) and network multiplayer.

## License

MIT. See [LICENSE](LICENSE).

## Author

Miguel Mochizuki Silva
