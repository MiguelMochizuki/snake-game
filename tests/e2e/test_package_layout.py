"""
test_package_layout.py
Author: Miguel Mochizuki Silva
Description: Runs tools/package.sh against a stub binary and checks the app layouts it produces
"""
import os
import plistlib
import shutil
import stat
import subprocess
import sys
import tempfile
import zipfile

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
SCRIPT = os.path.join(ROOT, "tools", "package.sh")


def package(target, extra_env=None):
    """Runs package.sh for a target with a stub binary and returns (out_dir, tmp_dir)."""
    tmp = tempfile.mkdtemp()
    binary = os.path.join(tmp, "snake-gui")
    with open(binary, "w") as f:
        f.write("#!/bin/sh\nexit 0\n")
    os.chmod(binary, 0o755)
    out = os.path.join(tmp, "out")
    env = {**os.environ, "NO_APPIMAGE": "1", "NO_ZIP": "1", **(extra_env or {})}
    subprocess.run(["bash", SCRIPT, target, binary, "9.9.9", out], check=True, env=env)
    return out, tmp


def test_linux_appdir():
    out, tmp = package("linux")
    d = os.path.join(out, "Snake.AppDir")
    for rel in ("AppRun", "snake-gui.desktop", "snake.png", ".DirIcon", "usr/bin/snake-gui"):
        assert os.path.exists(os.path.join(d, rel)), rel
    assert os.stat(os.path.join(d, "usr/bin/snake-gui")).st_mode & stat.S_IXUSR
    shutil.rmtree(tmp)


def test_macos_bundle():
    out, tmp = package("macos")
    app = os.path.join(out, "Snake.app", "Contents")
    assert os.access(os.path.join(app, "MacOS", "snake-gui"), os.X_OK)
    assert os.path.exists(os.path.join(app, "Resources", "icon.icns"))
    with open(os.path.join(app, "Info.plist"), "rb") as f:
        assert plistlib.load(f)["CFBundleShortVersionString"] == "9.9.9"
    shutil.rmtree(tmp)


def test_windows_zip():
    out, tmp = package("windows", {"NO_ZIP": ""})
    names = zipfile.ZipFile(os.path.join(out, "snake-gui-windows.zip")).namelist()
    assert "snake-gui.exe" in names and "README.txt" in names, names
    shutil.rmtree(tmp)


if __name__ == "__main__":
    for t in (test_linux_appdir, test_macos_bundle, test_windows_zip):
        t()
    print("ok")
    sys.exit(0)
