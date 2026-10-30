# Snake Game

The classic Snake game for the terminal, written in C++ with CMake. It saves your scores between sessions.

## Quick start

```bash
docker pull miguelmochizuki/snake-game
docker run --rm -it miguelmochizuki/snake-game
```

Scores stay inside the container and are lost when it exits. To keep them, mount a volume:

```bash
docker run --rm -it -v $(pwd)/data:/app/data miguelmochizuki/snake-game
```

## How to play

The game opens on a menu. Use W/S or the up/down arrows to move, A/D or left/right to change the level (Easy, Normal, Hard), and Enter to pick Start or Quit. The menu also shows your best score, games played and average score.

| Key | In the game |
|-----|-------------|
| W or Up arrow | Move up |
| A or Left arrow | Move left |
| S or Down arrow | Move down |
| D or Right arrow | Move right |
| Q or Ctrl+C | End the game |

When a game ends, press **R** to play again or **Q** to quit.

Set `NO_COLOR=1` to turn colors off.

## Platforms

The image supports linux/amd64 and linux/arm64.

## Security

The container runs as the non-root user `snakeuser` on a minimal Alpine base. A multi-stage build keeps only the runtime libraries in the final image.

## Source

The source code and full documentation are at [github.com/miguelmochizuki/snake-game](https://github.com/miguelmochizuki/snake-game).
