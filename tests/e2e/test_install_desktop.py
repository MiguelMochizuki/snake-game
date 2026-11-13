"""
test_install_desktop.py
Author: Miguel Mochizuki Silva
Description: Runs tools/install_desktop.sh into a temporary data directory and checks the desktop entry and icon
"""
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
SCRIPT = os.path.join(ROOT, "tools", "install_desktop.sh")


def run(*args):
    """Runs the script with the given arguments, failing the test on a non-zero exit."""
    subprocess.run(["bash", SCRIPT, *args], check=True, stdout=subprocess.DEVNULL)


def test_install_and_uninstall():
    """
    Checks install copies the binary, desktop entry and icon (even with a space in the path) and uninstall removes them
    """
    data = tempfile.mkdtemp()
    binary = os.path.join(data, "my game", "snake-gui")  # a space in the path must survive
    os.makedirs(os.path.dirname(binary))
    open(binary, "w").close()
    bindir = os.path.join(data, "bin")
    run("install", binary, data, bindir)
    installed = os.path.join(bindir, "snake-gui")
    assert os.access(installed, os.X_OK), "the binary is copied, so the repo can go away"
    entry = os.path.join(data, "applications", "snake-gui.desktop")
    kv = dict(l.split("=", 1) for l in open(entry, encoding="utf-8").read().splitlines() if "=" in l)
    assert kv["Exec"] == '"%s"' % installed, kv["Exec"]
    assert kv["Icon"] == "snake" and kv["Type"] == "Application" and kv["Name"] == "Snake"
    icon = os.path.join(data, "icons", "hicolor", "512x512", "apps", "snake.png")
    assert open(icon, "rb").read(8) == b"\x89PNG\r\n\x1a\n"
    shutil.rmtree(os.path.dirname(binary))  # delete the "repo": the entry must still point at something
    assert os.path.exists(kv["Exec"].strip('"'))
    run("uninstall", binary, data, bindir)
    assert not os.path.exists(entry) and not os.path.exists(icon) and not os.path.exists(installed)
    shutil.rmtree(data)


if __name__ == "__main__":
    test_install_and_uninstall()
    print("ok")
    sys.exit(0)
