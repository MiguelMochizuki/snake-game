"""
harness.py
Author: Miguel Mochizuki Silva
Description: Pty harness for the Linux end-to-end tests (spawns the game, feeds keys, parses frames)
"""
import fcntl
import os
import re
import select
import signal
import struct
import subprocess
import termios
import time

ANSI_RE = re.compile(r"\033\[[0-9;]*[A-Za-z]")
FRAME_END = "Controls: WASD to move, Q to quit"

# Keys with no effect in any state: function/navigation keys, modified keys, control bytes, UTF-8, other characters
UNREGISTERED_KEYS = [
	b"\x1bO2Q", b"\x1bO5D",
	b"\x1bOP", b"\x1b[15~", b"\x1b[24~",
	b"\x1b[H", b"\x1b[F",
	b"\x1b[5~", b"\x1b[6~",
	b"\x1b[2~", b"\x1b[3~",
	b"\x1b[1;5A",
	b"\x1bq", b"\x1bd",
	b"\x1a", b"\x1c", b"\x13", b"\x11",
	b"\xc3\xa9",
	b"\t", b"\r",
	b"x", b"1", b" ",
]


def scale():
	"""
	Reads the timeout multiplier from SNAKE_E2E_TIMEOUT_SCALE

	Returns:
		float: The multiplier (default 1.0)
	"""
	return float(os.environ.get("SNAKE_E2E_TIMEOUT_SCALE") or 1.0)


def strip_ansi(s):
	"""
	Removes ANSI CSI sequences, carriage returns and bell characters

	Args:
		s (str): Raw terminal output

	Returns:
		str: The text without escape sequences
	"""
	return ANSI_RE.sub("", s).replace("\r", "").replace("\a", "")


class Game:
	"""
	Runs the snake binary on a pty; use as a context manager so the child is always killed

	Attributes:
	subprocess.Popen proc: The child process
	int master: Pty master fd (our side)
	int slave: Pty slave fd (the child's terminal, kept open to read termios after exit)
	str out: Everything the child printed so far (raw)
	int pos: Offset in the stripped output where the next wait_for starts
	"""

	def __init__(self, binary, cwd=None, env=None, frame_ms=30, seed=1):
		"""
		Spawns the game

		Args:
			binary (str): Path to the snake executable
			cwd (str): Working directory (score files go to cwd/data)
			env (dict): Extra environment variables, applied after the SNAKE_* defaults (SNAKE_SKIP_MENU defaults to "1"; pass "" to see the menu)
			frame_ms (int): Frame time in ms; SNAKE_FRAME_MS gets frame_ms * scale() so slow hosts slow the game too
			seed (int): Value for SNAKE_SEED
		"""
		full_env = dict(os.environ, SNAKE_FRAME_MS=str(int(frame_ms * scale())), SNAKE_SEED=str(seed), SNAKE_SKIP_MENU="1")
		full_env.update(env or {})
		self.master, self.slave = os.openpty()
		self.out = ""
		self.pos = 0
		self.proc = subprocess.Popen([binary], cwd=cwd, env=full_env, stdin=self.slave,
			stdout=self.slave, stderr=self.slave, start_new_session=True)

	def __enter__(self):
		"""
		Enters the context manager

		Returns:
			Game: self
		"""
		return self

	def __exit__(self, *exc):
		"""
		Kills the child if still running and closes both pty fds

		Returns:
			bool: False (exceptions propagate)
		"""
		if self.proc.poll() is None:
			os.killpg(self.proc.pid, signal.SIGKILL)
			self.proc.wait()
		os.close(self.master)
		os.close(self.slave)
		return False

	def _pump(self, wait):
		"""
		Reads whatever the child printed, waiting up to `wait` seconds for data

		Args:
			wait (float): Maximum seconds to block in select
		"""
		ready, _, _ = select.select([self.master], [], [], wait)
		if ready:
			self.out += os.read(self.master, 65536).decode("latin-1")

	def _until(self, cond, timeout, what):
		"""
		Pumps output until cond() is truthy

		Args:
			cond (callable): Predicate polled after every read
			timeout (float): Seconds before scale()
			what (str): Description used in the failure message

		Returns:
			object: The truthy value cond() returned Throws AssertionError: On timeout, with the last 2 KB of stripped output
		"""
		deadline = time.monotonic() + timeout * scale()
		while True:
			result = cond()
			if result:
				return result
			left = deadline - time.monotonic()
			if left <= 0:
				raise AssertionError("timed out waiting for %s; output tail:\n%s"
					% (what, strip_ansi(self.out)[-2048:]))
			self._pump(min(left, 0.05))

	def send(self, keys):
		"""
		Sends keys to the game. TerminalKeySource (built in main before frame 1) switches the tty
		to raw mode with TCSAFLUSH, discarding anything typed earlier, so this first waits for
		frame 2 as a margin (free once the game is past it)

		Args:
			keys (str | bytes): Characters to type (a str is UTF-8 encoded, bytes are written as-is)
		"""
		self.wait_frames(2)
		os.write(self.master, keys.encode() if isinstance(keys, str) else keys)

	def send_now(self, keys):
		"""
		Sends keys without waiting for a game frame, for screens that are not the board (the start menu).
		Wait for something the screen printed first: the terminal is in raw mode by then, so the keys are not discarded

		Args:
			keys (str | bytes): Characters to type (a str is UTF-8 encoded, bytes are written as-is)
		"""
		os.write(self.master, keys.encode() if isinstance(keys, str) else keys)

	def drain(self):
		"""
		Reads all output already queued on the pty, so frame_count() is current
		"""
		while select.select([self.master], [], [], 0)[0]:
			self.out += os.read(self.master, 65536).decode("latin-1")

	def wait_input_consumed(self, timeout=5.0):
		"""
		Waits until the game has read every key sent so far (the slave's input queue is empty)

		Args:
			timeout (float): Seconds before scale()
			AssertionError (Throws): On timeout
		"""
		def empty():
			buf = fcntl.ioctl(self.slave, termios.FIONREAD, struct.pack("i", 0))
			return struct.unpack("i", buf)[0] == 0
		self._until(empty, timeout, "the game to read pending input")

	def wait_for(self, pattern, timeout=5.0):
		"""
		Waits for a regex in the stripped output after the previous match, and consumes it

		Args:
			pattern (str | re.Pattern): What to look for
			timeout (float): Seconds before scale()

		Returns:
			str: The matched text Throws AssertionError: On timeout
		"""
		rx = re.compile(pattern) if isinstance(pattern, str) else pattern
		m = self._until(lambda: rx.search(strip_ansi(self.out), self.pos), timeout, repr(rx.pattern))
		self.pos = m.end()
		return m.group(0)

	def frame_count(self):
		"""
		Counts the frames rendered so far

		Returns:
			int: Number of fully rendered frames so far
		"""
		return self.out.count(FRAME_END)

	def wait_frames(self, n, timeout=5.0):
		"""
		Waits until at least n frames were rendered in total

		Args:
			n (int): Frame count to reach
			timeout (float): Seconds before scale()
			AssertionError (Throws): On timeout
		"""
		self._until(lambda: self.frame_count() >= n, timeout, "%d frames" % n)

	def frames(self):
		"""
		Splits the output into complete frames

		Returns:
			list[list[str]]: Every complete frame as its 20 inner board rows (borders removed)
		"""
		result = []
		for chunk in self.out.split("\033[H"):
			if FRAME_END in chunk:
				lines = strip_ansi(chunk).split("\n")
				result.append([row[1:-1] for row in lines[1:21]])
		return result

	def last_frame(self):
		"""
		Gets the board of the last complete frame

		Returns:
			list[str]: The board rows of the last complete frame
		"""
		return self.frames()[-1]

	def find(self, ch):
		"""
		Locates a character in the last frame

		Args:
			ch (str): Character to find

		Returns:
			tuple: (x, y) board coordinates Throws AssertionError: If the character is not on the board
		"""
		for y, row in enumerate(self.last_frame()):
			if ch in row:
				return row.index(ch), y
		raise AssertionError("%r not on board:\n%s" % (ch, "\n".join(self.last_frame())))

	def exit_code(self, timeout=5.0):
		"""
		Waits for the child to exit while draining its output

		Args:
			timeout (float): Seconds before scale()

		Returns:
			int: The exit status Throws AssertionError: On timeout
		"""
		self._until(lambda: self.proc.poll() is not None, timeout, "process exit")
		self._pump(0)
		return self.proc.returncode

	def tty_attrs(self):
		"""
		Reads the termios attributes of the child's terminal

		Returns:
			list: termios attributes of the child's terminal (the slave fd)
		"""
		return termios.tcgetattr(self.slave)


def quit_game(game):
	"""
	Quits a running game from the board: q, wait for the panel, q, expect exit 0

	Args:
		game (Game): The running game

	Returns:
		str: Stripped output from the game-over panel on Throws AssertionError: If the panel never shows or the exit status is not 0
	"""
	game.send("q")
	game.wait_for("GAME OVER!")
	start = game.pos
	game.send("q")
	code = game.exit_code()
	assert code == 0, "exit code %d" % code
	return strip_ansi(game.out)[start:]


def play_once(binary, cwd):
	"""
	Plays one game that quits right away with score 0

	Args:
		binary (str): Path to the snake executable
		cwd (str): Working directory

	Returns:
		str: Stripped game-over panel text
	"""
	with Game(binary, cwd=cwd) as game:
		game.wait_for(r"Score: 0")
		return quit_game(game)
