# Snake Game

A terminal-based implementation of the classic Snake game in C++ with persistent score tracking and Docker support.

## Table of Contents
- [Features](#-features)
- [Downloads](#-downloads)
- [Project Structure](#-project-structure)
- [Prerequisites](#-prerequisites)
- [Installation](#-installation)
  - [Local Build](#local-build)
  - [Docker Build](#docker-build)
- [Usage](#-usage)
- [Game Controls](#-game-controls)
- [Score Management](#-score-management)
- [Building with CMake](#-building-with-cmake)
- [CI/CD](#-cicd)
- [License](#-license)
- [Author](#-author)

## Features

- **Classic Snake Gameplay** - Control a snake to eat food and grow
- **Score Persistence** - Best score and game history are saved between sessions
- **Terminal-Based Rendering** - Clean, real-time display using ANSI escape codes
- **Cross-Platform Support** - Works on Linux, macOS, and Windows
- **Pre-built Binaries** - Download ready-to-run executables for all platforms
- **Docker Support** - Containerized deployment with persistent data volumes
- **Non-Blocking Input** - Smooth gameplay with WASD controls
- **Game Statistics** - Tracks total games played and average score

## Downloads

Pre-built binaries are available for download from the [Releases](https://github.com/miguelmochizuki/snake-game/releases) page:

| Platform | Architecture | Binary |
|----------|-------------|--------|
| Linux | x86-64 | `snake-linux-x86_64` |
| Linux | ARM64 | `snake-linux-arm64` |
| Windows | x86-64 | `snake-windows-x86_64.exe` |
| Windows | ARM64 | `snake-windows-arm64.exe` |
| macOS | Universal (x86_64 + ARM64) | `snake-macos-universal` |

### Running Downloaded Binaries

**Linux/macOS:**
```bash
# Make executable
chmod +x snake-*

# Run
./snake-linux-x86_64
```

**Windows:**
```cmd
# Double-click or run from command prompt
snake-windows-x86_64.exe
```

## Project Structure

```
snake-game/
├── CMakeLists.txt          # CMake build configuration
├── Dockerfile              # Multi-stage Docker build
├── DOCKER_HUB.md           # Docker Hub repository description
├── Makefile                # Build orchestration
├── LICENSE                 # MIT License
├── .github/
│   └── workflows/
│       └── build-and-release.yml  # CI build, release & Docker publish
├── data/                   # Score persistence directory
│   ├── best_score.txt     # Best score storage
│   └── score_history.txt  # All scores history
├── include/                # Header files
│   ├── entity.hpp         # Abstract base class for entities
│   ├── food.hpp           # Food entity
│   ├── game_engine.hpp    # Main game loop orchestrator
│   ├── game_state.hpp     # Complete game state management
│   ├── score_manager.hpp  # Score persistence
│   ├── snake.hpp          # Snake entity
│   ├── snake_segment.hpp  # Snake body segment
│   ├── terminal_renderer.hpp # Terminal rendering
│   └── wasd_input_handler.hpp # Input handling
└── src/                    # Source files
    ├── main.cpp           # Program entry point
    ├── entity.cpp
    ├── food.cpp
    ├── game_engine.cpp
    ├── game_state.cpp
    ├── score_manager.cpp
    ├── snake.cpp
    ├── snake_segment.cpp
    ├── terminal_renderer.cpp
    └── wasd_input_handler.cpp
```

## Prerequisites

### Local Build
- **C++17 Compiler** (g++ 7+, clang 5+, or MSVC 2017+)
- **CMake** 3.10 or higher
- **Make** (or Ninja)
- **POSIX-compliant system** (Linux/macOS) or Windows with MinGW/MSVC

### Docker Build
- **Docker** 20.10 or higher

## Installation

### Local Build

#### Using Make (Recommended)
```bash
# Build the game
make local

# Build and run
make run-local

# Clean build artifacts
make clean-local

# Rebuild from scratch
make rebuild-local
```

#### Using CMake Directly
```bash
mkdir build && cd build
cmake ..
make
./bin/snake
```

#### Platform-Specific Notes

**Linux/macOS:**
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake

# Install dependencies (macOS with Homebrew)
brew install cmake
```

**Windows (MinGW/MSVC):**
```bash
# Using CMake with MinGW
cmake -G "MinGW Makefiles" ..
mingw32-make
./bin/snake.exe
```

### Docker Build

#### Using Make
```bash
# Build Docker image
make docker

# Run in Docker with persistent scores
make run-docker

# Clean Docker artifacts
make clean-docker

# Rebuild Docker image
make rebuild-docker
```

#### Using Docker Directly
```bash
# Build the image
docker build -t snake-game .

# Run with volume for persistence
docker run --rm -it -v $(pwd)/data:/app/data snake-game
```

## Usage

### Starting the Game
```bash
# Local
make run-local

# Docker
make run-docker

# Downloaded binary
./snake-<platform>
```

### Game Controls
- **W** - Move Up
- **A** - Move Left
- **S** - Move Down
- **D** - Move Right
- **Q** - Quit Game

### Game Over Screen
After the game ends, you'll see:
- Final Score
- Best Score
- Total Games Played
- Average Score

Press **R** to restart or **Q** to quit.

## Score Management

The game automatically saves scores to the `data/` directory:

- `best_score.txt` - Stores your highest score
- `score_history.txt` - Records all scores from every game session

### Viewing Scores
```bash
# View best score
cat data/best_score.txt

# View score history
cat data/score_history.txt
```

## Building with CMake

### Build Types
```bash
# Release build (default)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# RelWithDebInfo build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

### CMake Options
| Option | Description | Default |
|--------|-------------|---------|
| `CMAKE_BUILD_TYPE` | Build configuration | Release |
| `CMAKE_CXX_STANDARD` | C++ standard version | 17 |

### Compiler Warnings
The project enables strict compiler warnings:
- GCC/Clang: `-Wall -Wextra -Wpedantic`
- MSVC: `/W4`

## Docker Details

### Multi-Stage Build
The Dockerfile uses a multi-stage build for minimal image size:
1. **Builder Stage**: Alpine with build tools (CMake, g++)
2. **Runtime Stage**: Minimal Alpine with runtime libraries

### Volume Persistence
```bash
# The data directory is persisted via volume
-v $(pwd)/data:/app/data
```

### Pre-built Image on Docker Hub
A pre-built Docker image is available on Docker Hub at [miguelmochizuki/snake-game](https://hub.docker.com/repository/docker/miguelmochizuki/snake-game/general):
```bash
docker pull miguelmochizuki/snake-game
docker run --rm -it -v $(pwd)/data:/app/data miguelmochizuki/snake-game
```

### Security
- Runs as non-root user (`snakeuser`)
- Minimal Alpine base image
- Only necessary runtime dependencies

## CI/CD

The project uses a single GitHub Actions workflow for continuous integration and delivery:

### Build
On every push to `main` or pull request, binaries are built for all supported platforms:
- Linux x86-64
- Linux ARM64
- Windows x86-64
- Windows ARM64
- macOS Universal

### Release
When a tag matching `v*` is pushed, the workflow additionally:
1. Creates a GitHub Release attaching all pre-built binaries
2. Builds and publishes a multi-platform Docker image to Docker Hub

Because `docker-publish` runs in parallel with `build`, the Docker image is built **while** platform binaries are being compiled.

### Creating a New Release
```bash
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

The release and Docker image will be created automatically.

### Docker Hub
A pre-built multi-platform image (linux/amd64, linux/arm64) is available at:
```bash
docker pull miguelmochizuki/snake-game
docker run --rm -it -v $(pwd)/data:/app/data miguelmochizuki/snake-game
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Miguel Mochizuki Silva**

---

## Additional Notes

### Troubleshooting

**Issue**: Terminal input not working correctly
- Ensure your terminal supports raw input mode
- On Windows, use a terminal that supports ANSI escape codes (Windows Terminal, ConEmu)

**Issue**: Docker permission errors
```bash
# Add your user to the docker group
sudo usermod -aG docker $USER
# Log out and back in
```

**Issue**: Scores not persisting in Docker
- Ensure the `data/` directory is mounted as a volume
- Check permissions: `chmod 755 data/`

### Development

To contribute or modify the game:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with `make run-docker`
5. Submit a pull request

### Future Enhancements
- [ ] Multiple difficulty levels
- [ ] Color-coded rendering
- [ ] Leaderboard system
- [ ] Sound effects (terminal beep)
- [ ] Network multiplayer mode

---

**Enjoy the game! 🐍**
