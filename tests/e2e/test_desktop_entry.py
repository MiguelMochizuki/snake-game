"""
test_desktop_entry.py
Author: Miguel Mochizuki Silva
Description: Checks the Linux .desktop entry, the macOS Info.plist template and the Windows icon resource
"""
import os
import plistlib
import sys

ASSETS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "assets")


def read(name):
    """Returns the text of an asset file, failing the test when it is missing."""
    with open(os.path.join(ASSETS, name), encoding="utf-8") as f:
        return f.read()


def test_desktop():
    kv = dict(l.split("=", 1) for l in read("snake-gui.desktop").splitlines() if "=" in l)
    assert kv["Type"] == "Application"
    assert kv["Exec"] == "snake-gui"
    assert kv["Icon"] == "snake"
    assert "Game" in kv["Categories"].split(";")
    assert kv["Name"] == "Snake"


def test_plist():
    plist = plistlib.loads(read("Info.plist.in").replace("@PROJECT_VERSION@", "9.9.9").encode())
    assert plist["CFBundleExecutable"] == "snake-gui"
    assert plist["CFBundleIconFile"] == "icon"
    assert plist["CFBundleIdentifier"] == "io.github.miguelmochizuki.snake"
    assert plist["NSHighResolutionCapable"] is True
    assert plist["CFBundleShortVersionString"] == "9.9.9"


def test_rc():
    assert "icon.ico" in read("snake-gui.rc")


if __name__ == "__main__":
    for t in (test_desktop, test_plist, test_rc):
        t()
    print("ok")
    sys.exit(0)
