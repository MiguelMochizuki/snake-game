# Snake Game

A terminal-based classic Snake game with persistent score tracking. Built in C++ with CMake.

## Quick Start

```bash
docker pull miguelmochizuki/snake-game
docker run --rm -it miguelmochizuki/snake-game
```

Scores are saved inside the container but are lost by default. To persist scores between runs, mount a volume:

```bash
docker run --rm -it -v $(pwd)/data:/app/data miguelmochizuki/snake-game
```

## Game Controls

| Key | Action |
|-----|--------|
| W | Move Up |
| A | Move Left |
| S | Move Down |
| D | Move Right |
| Q | Quit |

After game over, press **R** to restart or **Q** to quit.

## Features

- Classic snake gameplay with real-time terminal rendering
- Persistent score tracking (best score + game history)
- Smooth WASD controls with non-blocking input
- Game statistics (total games, average score)

## Platforms

This image supports both **linux/amd64** and **linux/arm64** architectures.

## Security

- Runs as non-root user `snakeuser`
- Minimal Alpine base image
- Only runtime libraries included (multi-stage build)

## Source

Source code and full documentation available at [github.com/miguelmochizuki/snake-game](https://github.com/miguelmochizuki/snake-game)
