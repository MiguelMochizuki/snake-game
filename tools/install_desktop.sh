#!/usr/bin/env bash
# install_desktop.sh
# Author: Miguel Mochizuki Silva
# Description: Installs (or removes) the snake-gui desktop entry and icon for the current user, so a build
# run from a folder gets its icon in the dock and app menu. GNOME on Wayland takes the icon from the entry.
# Usage: tools/install_desktop.sh <install|uninstall> <snake-gui binary> [data home]
# The data home defaults to $XDG_DATA_HOME, or ~/.local/share.
set -euo pipefail

action=$1 binary=$2
data=${3:-${XDG_DATA_HOME:-$HOME/.local/share}}
root=$(cd "$(dirname "$0")/.." && pwd)
entry="$data/applications/snake-gui.desktop"
icon="$data/icons/hicolor/512x512/apps/snake.png"

case "$action" in
install)
	mkdir -p "$data/applications" "$(dirname "$icon")"
	abs=$(cd "$(dirname "$binary")" && pwd)/$(basename "$binary")
	# Desktop entries need a path with spaces in double quotes
	sed "s|^Exec=.*|Exec=\"$abs\"|" "$root/assets/snake-gui.desktop" >"$entry"
	cp "$root/assets/icon-512.png" "$icon"
	;;
uninstall)
	rm -f "$entry" "$icon"
	;;
*)
	echo "usage: $0 <install|uninstall> <snake-gui binary> [data home]" >&2
	exit 2
	;;
esac
# Refresh the desktop's caches when their tools exist; harmless when they do not
update-desktop-database "$data/applications" 2>/dev/null || true
gtk-update-icon-cache -q -f -t "$data/icons/hicolor" 2>/dev/null || true
