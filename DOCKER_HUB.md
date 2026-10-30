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

## Start Menu

The game opens on a menu. Use **W**/**S** (or the arrow keys) to move, **A**/**D** (or Left/Right) to change the level (Easy, Normal, Hard), and **Enter** to select Start or Quit. The menu also shows your best score, games played and average score.

## Game Controls

| Key | Action |
|-----|--------|
| W or Up arrow | Move Up |
| A or Left arrow | Move Left |
| S or Down arrow | Move Down |
| D or Right arrow | Move Right |
| Q or Ctrl+C | Quit |

After game over, press **R** to restart or **Q** to quit.

## Features

- Classic snake gameplay with real-time terminal rendering
- Persistent score tracking (best score + game history)
- Start menu with Easy, Normal and Hard levels
- Smooth WASD or arrow controls with non-blocking input
- Game statistics (total games, average score)

## Platforms

This image supports both **linux/amd64** and **linux/arm64** architectures.

## Security

- Runs as non-root user `snakeuser`
- Minimal Alpine base image
- Only runtime libraries included (multi-stage build)

## Source

Source code and full documentation available at [github.com/miguelmochizuki/snake-game](https://github.com/miguelmochizuki/snake-game)
