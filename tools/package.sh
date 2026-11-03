#!/usr/bin/env bash
# package.sh
# Author: Miguel Mochizuki Silva
# Description: Packages snake-gui for one OS: a Windows zip, a macOS Snake.app zip or a Linux AppImage
# Usage: tools/package.sh <windows|macos|linux> <snake-gui binary> <version> <out dir>
# Env: NO_ZIP=1 skips zipping, NO_APPIMAGE=1 skips appimagetool (layouts are still built, for tests)
set -euo pipefail

target=$1 binary=$2 version=$3 out=$4
root=$(cd "$(dirname "$0")/.." && pwd)
assets="$root/assets"
mkdir -p "$out"

case "$target" in
windows)
	stage="$out/stage"
	rm -rf "$stage" && mkdir -p "$stage"
	cp "$binary" "$stage/snake-gui.exe"
	printf 'Snake %s\r\n\r\nDouble-click snake-gui.exe to play.\r\nWindows SmartScreen may say "unknown publisher": choose More info, then Run anyway.\r\n' "$version" >"$stage/README.txt"
	if [ -z "${NO_ZIP:-}" ]; then
		python3 - "$stage" "$out/snake-gui-windows.zip" <<'PY'
import os, sys, zipfile
src, dst = sys.argv[1:3]
with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as z:
    for n in sorted(os.listdir(src)):
        z.write(os.path.join(src, n), n)
PY
	fi
	;;
macos)
	app="$out/Snake.app/Contents"
	rm -rf "$out/Snake.app" && mkdir -p "$app/MacOS" "$app/Resources"
	cp "$binary" "$app/MacOS/snake-gui" && chmod +x "$app/MacOS/snake-gui"
	cp "$assets/icon.icns" "$app/Resources/icon.icns"
	sed "s/@PROJECT_VERSION@/$version/g" "$assets/Info.plist.in" >"$app/Info.plist"
	if [ -z "${NO_ZIP:-}" ]; then
		ditto -c -k --keepParent "$out/Snake.app" "$out/snake-gui-macos.zip"
	fi
	;;
linux)
	d="$out/Snake.AppDir"
	rm -rf "$d" && mkdir -p "$d/usr/bin"
	cp "$binary" "$d/usr/bin/snake-gui" && chmod +x "$d/usr/bin/snake-gui"
	cp "$assets/snake-gui.desktop" "$d/snake-gui.desktop"
	cp "$assets/icon.png" "$d/snake.png"
	cp "$assets/icon.png" "$d/.DirIcon"
	printf '#!/bin/sh\nexec "$(dirname "$(readlink -f "$0")")/usr/bin/snake-gui" "$@"\n' >"$d/AppRun"
	chmod +x "$d/AppRun"
	if [ -z "${NO_APPIMAGE:-}" ]; then
		arch=$(uname -m)
		ARCH=$arch appimagetool --appimage-extract-and-run "$d" "$out/Snake-$arch.AppImage"
	fi
	;;
*)
	echo "unknown target: $target" >&2
	exit 2
	;;
esac
