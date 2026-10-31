"""
test_icon_assets.py
Author: Miguel Mochizuki Silva
Description: Checks the committed app icons (PNG, ICO, ICNS) without needing Pillow
"""
import os
import struct
import sys
import zlib

ASSETS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "assets")


def read(name):
    """Returns the bytes of an icon file, failing the test when it is missing."""
    with open(os.path.join(ASSETS, name), "rb") as f:
        return f.read()


def png_pixels(data):
    """Decodes an 8-bit RGBA non-interlaced PNG into (width, height, rows of bytes)."""
    assert data[:8] == b"\x89PNG\r\n\x1a\n", "not a PNG"
    pos, idat = 8, b""
    while pos < len(data):
        n, kind = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + n]
        if kind == b"IHDR":
            w, h, depth, ctype, _, _, interlace = struct.unpack(">IIBBBBB", body)
            assert (depth, ctype, interlace) == (8, 6, 0), "need 8-bit RGBA, not interlaced"
        elif kind == b"IDAT":
            idat += body
        pos += 12 + n
    raw, stride, rows, prev = zlib.decompress(idat), w * 4, [], bytearray(w * 4)
    for y in range(h):
        base = y * (stride + 1)
        ft, line = raw[base], bytearray(raw[base + 1:base + 1 + stride])
        for i in range(stride):
            a = line[i - 4] if i >= 4 else 0
            b = prev[i]
            c = prev[i - 4] if i >= 4 else 0
            if ft == 1:
                line[i] = (line[i] + a) & 255
            elif ft == 2:
                line[i] = (line[i] + b) & 255
            elif ft == 3:
                line[i] = (line[i] + (a + b) // 2) & 255
            elif ft == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        rows.append(line)
        prev = line
    return w, h, rows


def test_png():
    w, h, rows = png_pixels(read("icon.png"))
    assert (w, h) == (1024, 1024), (w, h)
    assert rows[0][3] == 0, "corner must be transparent (rounded square)"
    assert rows[h // 2][(w // 2) * 4 + 3] == 255, "centre must be opaque"


def test_ico():
    data = read("icon.ico")
    reserved, kind, count = struct.unpack("<HHH", data[:6])
    assert (reserved, kind) == (0, 1) and count >= 6, (kind, count)


def test_icns():
    assert read("icon.icns")[:4] == b"icns"


if __name__ == "__main__":
    for t in (test_png, test_ico, test_icns):
        t()
    print("ok")
    sys.exit(0)
