#!/usr/bin/env bash
# install_desktop.sh
# Author: Miguel Mochizuki Silva
# Description: Installs (or removes) snake-gui for the current user on Linux: the binary, a desktop entry and
# the icon, so the game shows up in the app menu and dock and keeps working if the source folder is deleted.
# GNOME on Wayland takes the dock icon from the entry.
# Usage: tools/install_desktop.sh <install|uninstall> <snake-gui binary> [data home] [bin dir]
# The data home defaults to $XDG_DATA_HOME, or ~/.local/share; the bin dir to ~/.local/bin.
set -euo pipefail

action=$1 binary=$2
data=${3:-${XDG_DATA_HOME:-$HOME/.local/share}}
bindir=${4:-$HOME/.local/bin}
root=$(cd "$(dirname "$0")/.." && pwd)
entry="$data/applications/snake-gui.desktop"
icon="$data/icons/hicolor/512x512/apps/snake.png"
installed="$bindir/snake-gui"

case "$action" in
install)
	mkdir -p "$data/applications" "$(dirname "$icon")" "$bindir"
	install -m 755 "$binary" "$installed"
	# Desktop entries need a path with spaces in double quotes
	sed "s|^Exec=.*|Exec=\"$installed\"|" "$root/assets/snake-gui.desktop" >"$entry"
	cp "$root/assets/icon-512.png" "$icon"
	;;
uninstall)
	rm -f "$entry" "$icon" "$installed"
	;;
*)
	echo "usage: $0 <install|uninstall> <snake-gui binary> [data home] [bin dir]" >&2
	exit 2
	;;
esac
# Refresh the desktop's caches when their tools exist; harmless when they do not
update-desktop-database "$data/applications" 2>/dev/null || true
gtk-update-icon-cache -q -f -t "$data/icons/hicolor" 2>/dev/null || true
