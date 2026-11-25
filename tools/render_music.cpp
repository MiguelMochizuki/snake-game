/**
 * render_music.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Writes every piece of the soundtrack as a WAV file, played by the game's own synth, to listen to them
 * outside the game. Usage: snake_render_music OUT_DIR [melody|accompaniment]; with a stem, only those notes play
 * (tools/check_balance.py uses that to measure the melody against the accompaniment)
 */
#include "snake_game/audio/score.hpp"
#include "snake_game/audio/synth.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

/**
 * Writes mono 16-bit samples as a WAV file
 *
 * Parameters:
 * const std::string& path: File to write
 * const std::vector<int16_t>& samples: The samples
 * int rate: Sample rate in Hz
 */
static void writeWav(const std::string& path, const std::vector<int16_t>& samples, int rate) {
	FILE* f = std::fopen(path.c_str(), "wb");
	if (!f) {
		std::perror(path.c_str());
		std::exit(1);
	}
	const uint32_t bytes = static_cast<uint32_t>(samples.size() * 2);
	const uint32_t header[] = {0x46464952u, 36 + bytes, 0x45564157u, 0x20746d66u, 16u};
	const uint16_t fmt[] = {1, 1};
	const uint32_t rates[] = {static_cast<uint32_t>(rate), static_cast<uint32_t>(rate * 2)};
	const uint16_t align[] = {2, 16};
	const uint32_t data[] = {0x61746164u, bytes};
	std::fwrite(header, 4, 5, f);
	std::fwrite(fmt, 2, 2, f);
	std::fwrite(rates, 4, 2, f);
	std::fwrite(align, 2, 2, f);
	std::fwrite(data, 4, 2, f);
	std::fwrite(samples.data(), 2, samples.size(), f);
	std::fclose(f);
}

int main(int argc, char** argv) {
	if (argc != 2 && argc != 3) {
		std::fprintf(stderr, "usage: %s OUT_DIR [melody|accompaniment]\n", argv[0]);
		return 1;
	}
	if (argc == 3) {
		setenv("SNAKE_STEM", argv[2], 1);
	}
	const int rate = 44100;
	for (int i = 0; i < PIECE_COUNT; ++i) {
		// The synth plays the piece SNAKE_PIECE names, counted from 1
		setenv("SNAKE_PIECE", std::to_string(i + 1).c_str(), 1);
		Synth synth(rate);
		synth.startMusic();
		const int frames = static_cast<int>(static_cast<long long>(PIECES[i].endMs + 1000) * rate / 1000);
		std::vector<float> buffer(frames);
		synth.render(buffer.data(), frames);
		std::vector<int16_t> pcm(frames);
		for (int k = 0; k < frames; ++k) {
			const float v = buffer[k] < -1.f ? -1.f : (buffer[k] > 1.f ? 1.f : buffer[k]);
			pcm[k] = static_cast<int16_t>(v * 32767.f);
		}
		char name[256];
		std::snprintf(name, sizeof name, "%s/%02d-%s.wav", argv[1], i + 1, PIECES[i].title);
		for (char* c = name + std::string(argv[1]).size() + 1; *c; ++c) {
			if (*c == ' ' || *c == '/' || *c == ',') *c = '_';
		}
		writeWav(name, pcm, rate);
		std::printf("%s\n", name);
	}
	return 0;
}
