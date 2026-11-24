#!/usr/bin/env bash
# clean_user_data.sh
# Author: Miguel Mochizuki Silva
# Description: Removes the scores both versions keep for the current user (snake-game for the graphical game,
# snake-game-terminal for the terminal game) on Linux and macOS. The ./data folder of a source checkout is not
# touched: make clean-data is for that.
# Usage: tools/clean_user_data.sh [data home]
# The data home defaults to ~/Library/Application Support on macOS, else $XDG_DATA_HOME or ~/.local/share.
set -euo pipefail

if [ -n "${1:-}" ]; then
	base=$1
elif [ "$(uname -s)" = Darwin ]; then
	base="$HOME/Library/Application Support"
else
	base=${XDG_DATA_HOME:-$HOME/.local/share}
fi
for name in snake-game snake-game-terminal; do
	dir="$base/$name"
	if [ -d "$dir" ]; then
		echo "Removing $dir"
		rm -rf "$dir"
	fi
done
