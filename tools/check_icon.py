#!/usr/bin/env python3
"""
check_icon.py
Author: Miguel Mochizuki Silva
Description: Fails unless a packaged build carries its app icon (Windows exe resources, macOS icns)
Usage: check_icon.py exe <snake-gui.exe> | check_icon.py icns <icon.icns>
"""
import struct
import sys

RT_ICON, RT_GROUP_ICON = 3, 14


def pe_resource_types(data):
	"""
	Read the exe's top-level resource directory

	Args:
		data (bytes): Contents of the PE file

	Returns:
		dict: Resource type id -> number of entries
	"""
	pe = struct.unpack_from("<I", data, 0x3C)[0]
	if data[pe:pe + 4] != b"PE\0\0":
		raise SystemExit("not a PE file")
	nsec, = struct.unpack_from("<H", data, pe + 6)
	opt_size, = struct.unpack_from("<H", data, pe + 20)
	opt = pe + 24
	magic, = struct.unpack_from("<H", data, opt)
	dirs = opt + (112 if magic == 0x20B else 96)
	rva, size = struct.unpack_from("<II", data, dirs + 2 * 8)
	if not rva:
		raise SystemExit("exe has no resource section")
	sec = opt + opt_size

	def to_off(r):
		"""Map a resource RVA to a file offset"""
		for i in range(nsec):
			va, raw_size, raw = struct.unpack_from("<III", data, sec + i * 40 + 12)
			if va <= r < va + max(raw_size, 1):
				return r - va + raw
		raise SystemExit("resource RVA outside any section")

	root = to_off(rva)
	named, ids = struct.unpack_from("<HH", data, root + 12)
	types = {}
	for i in range(named + ids):
		name, off = struct.unpack_from("<II", data, root + 16 + i * 8)
		if off & 0x80000000 and not name & 0x80000000:
			sub = root + (off & 0x7FFFFFFF)
			n1, n2 = struct.unpack_from("<HH", data, sub + 12)
			types[name] = n1 + n2
	return types


def check_exe(path):
	"""
	Exit with an error unless the exe has icon group and image resources

	Args:
		path (str): Path to the exe
	"""
	types = pe_resource_types(open(path, "rb").read())
	if not types.get(RT_GROUP_ICON) or not types.get(RT_ICON):
		raise SystemExit(f"{path}: no icon resources (found types {sorted(types)})")
	print(f"{path}: {types[RT_GROUP_ICON]} icon group(s), {types[RT_ICON]} image(s)")


def check_icns(path):
	"""
	Exit with an error unless the icns is well formed and holds a 512px image

	Args:
		path (str): Path to the icns
	"""
	data = open(path, "rb").read()
	if data[:4] != b"icns" or struct.unpack(">I", data[4:8])[0] != len(data):
		raise SystemExit(f"{path}: not a well-formed icns")
	kinds, i = [], 8
	while i < len(data):
		n = struct.unpack(">I", data[i + 4:i + 8])[0]
		if n < 8:
			raise SystemExit(f"{path}: corrupt chunk")
		kinds.append(data[i:i + 4].decode("latin1"))
		i += n
	if not {"ic09", "ic10"} & set(kinds):  # 512px and 1024px (PNG)
		raise SystemExit(f"{path}: no 512px image (chunks {kinds})")
	print(f"{path}: chunks {kinds}")


if __name__ == "__main__":
	if len(sys.argv) != 3 or sys.argv[1] not in ("exe", "icns"):
		raise SystemExit(__doc__)
	(check_exe if sys.argv[1] == "exe" else check_icns)(sys.argv[2])
