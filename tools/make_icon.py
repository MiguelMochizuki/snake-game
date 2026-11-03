"""
make_icon.py
Author: Miguel Mochizuki Silva
Description: Draws the Snake app icon (coiled green snake beside a red dot on a dark
rounded square) and writes icon.png, icon.ico and icon.icns. Needs Pillow.
Usage: python3 tools/make_icon.py [--out assets]
"""
import argparse
import math
import os

from PIL import Image, ImageDraw

SIZE = 1024
SCALE = 4  # supersampling for smooth edges
BACKGROUND = (15, 18, 17)
SURFACE = (24, 30, 28)
BODY = (58, 148, 104)
HEAD = (124, 214, 162)
FOOD = (218, 98, 92)


def coil_points(cx, cy, turns, r_start, r_end, steps):
    """Returns points of a spiral from the outside in, as the snake's body path."""
    pts = []
    for i in range(steps + 1):
        t = i / steps
        a = -math.pi / 2 + t * turns * 2 * math.pi
        r = r_start + (r_end - r_start) * t
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


def draw_icon():
    """Draws the icon at SIZE x SIZE with a transparent rounded-square outside."""
    s = SIZE * SCALE
    img = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((0, 0, s - 1, s - 1), radius=int(s * 0.225), fill=BACKGROUND)
    d.rounded_rectangle((s * 0.03, s * 0.03, s * 0.97, s * 0.97), radius=int(s * 0.2), fill=SURFACE)

    width = s * 0.082
    pts = coil_points(s * 0.42, s * 0.52, 1.9, s * 0.30, s * 0.06, 400)
    for (x, y) in pts:  # round brush stamps give a smooth, round-capped body
        d.ellipse((x - width / 2, y - width / 2, x + width / 2, y + width / 2), fill=BODY)

    hx, hy = pts[0]  # outer end of the spiral is the head
    hr = width * 0.95
    d.ellipse((hx - hr, hy - hr, hx + hr, hy + hr), fill=HEAD)
    er = hr * 0.22
    d.ellipse((hx - er, hy - hr * 0.1 - er, hx + er, hy - hr * 0.1 + er), fill=BACKGROUND)

    fx, fy, fr = s * 0.80, s * 0.26, s * 0.062
    d.ellipse((fx - fr, fy - fr, fx + fr, fy + fr), fill=FOOD)
    return img.resize((SIZE, SIZE), Image.LANCZOS)


def main():
    """Parses --out and writes the three icon files there."""
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(os.path.dirname(__file__), "..", "assets"))
    out = ap.parse_args().out
    os.makedirs(out, exist_ok=True)
    img = draw_icon()
    img.save(os.path.join(out, "icon.png"))
    img.save(os.path.join(out, "icon.ico"), sizes=[(n, n) for n in (16, 32, 48, 64, 128, 256)])
    img.save(os.path.join(out, "icon.icns"))


if __name__ == "__main__":
    main()
