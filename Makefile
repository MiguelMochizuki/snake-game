# Snake Game Makefile
# Author: Miguel Mochizuki Silva
# Description: Builds, runs and tests the three variants (term, audio, gui), plus Docker and the maintenance tasks

# ==============================
# Configuration
# ==============================

CMAKE = cmake
CMAKE_BUILD_TYPE ?= Release
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# Each variant builds in its own directory: build directory, extra CMake flags, executable and environment of run
VARIANTS = term audio gui
DIR_term = build
DIR_audio = build-audio
DIR_gui = build-sdl
FLAGS_term =
FLAGS_audio = -DSNAKE_GAME_BUILD_AUDIO=ON
FLAGS_gui = -DSNAKE_GAME_BUILD_GUI=ON
EXE_term = snake
EXE_audio = snake
EXE_gui = snake-gui
ENV_term = SNAKE_DATA_DIR=data
ENV_audio = SNAKE_DATA_DIR=data
ENV_gui =

IMAGE = snake-game
VOLUME = snake-data

OS_NAME := $(shell uname -s)
APP_VERSION := $(shell sed -n 's/^project.*VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)
MAC_APP = $(HOME)/Applications/Snake.app

# Colors
GREEN = \033[0;32m
BLUE = \033[0;34m
YELLOW = \033[0;33m
NC = \033[0m

# say COLOR TEXT: a colored line
say = @printf '$(1)%s$(NC)\n' '$(2)'

# ==============================
# Variants: make X, make run-X, make test-X
# ==============================

define variant
.PHONY: $(1) run-$(1) test-$(1)

$(1):
	$$(call say,$$(BLUE),Building $(1)...)
	@$$(CMAKE) -S . -B $$(DIR_$(1)) $$(CMAKE_FLAGS) $$(FLAGS_$(1))
	@$$(CMAKE) --build $$(DIR_$(1)) -j
	$$(call say,$$(GREEN),Built $$(DIR_$(1))/bin/$$(EXE_$(1)))

run-$(1): $(1)
	$$(call say,$$(GREEN),Running $(1)...)
	@$$(ENV_$(1)) $$(DIR_$(1))/bin/$$(EXE_$(1))

test-$(1): $(1)
	$$(call say,$$(BLUE),Testing $(1)...)
	@ctest --test-dir $$(DIR_$(1)) --output-on-failure
endef

$(foreach v,$(VARIANTS),$(eval $(call variant,$(v))))

.DEFAULT_GOAL := all
.PHONY: all run test test-all rebuild
all: term
run: run-term
test: test-term
test-all: $(addprefix test-,$(VARIANTS))
rebuild: clean all

# ==============================
# Clean
# ==============================

.PHONY: clean clean-data clean-user-data

clean:
	$(call say,$(YELLOW),Cleaning build files...)
	@rm -rf $(foreach v,$(VARIANTS),$(DIR_$(v))) build-icon build-music

clean-data:
	$(call say,$(YELLOW),Removing local scores...)
	@rm -rf data

# Removes the scores both versions keep for your user (clean-data removes the ./data of this checkout)
clean-user-data:
ifeq ($(filter Darwin Linux,$(OS_NAME)),)
	$(call say,$(YELLOW),On Windows: delete %APPDATA%\snake-game and %APPDATA%\snake-game-terminal)
else
	@bash tools/clean_user_data.sh
endif

# ==============================
# Tools
# ==============================

.PHONY: install-gui uninstall-gui icon music loudness balance

# Installs the graphical game for your user (Linux: ~/.local/bin with menu entry and icon; macOS: ~/Applications)
install-gui: gui
ifeq ($(OS_NAME),Darwin)
	@tmp=$$(mktemp -d) && NO_ZIP=1 bash tools/package.sh macos $(DIR_gui)/bin/snake-gui $(APP_VERSION) $$tmp \
		&& mkdir -p $(dir $(MAC_APP)) && rm -rf $(MAC_APP) && cp -R $$tmp/Snake.app $(MAC_APP) && rm -rf $$tmp
	$(call say,$(GREEN),Installed $(MAC_APP))
else ifeq ($(OS_NAME),Linux)
	@bash tools/install_desktop.sh install $(DIR_gui)/bin/snake-gui
	$(call say,$(GREEN),Installed ~/.local/bin/snake-gui with its menu entry and icon; log out and in if your dock does not show it)
else
	$(call say,$(YELLOW),On Windows: download the zip from the Releases page)
endif

uninstall-gui:
ifeq ($(OS_NAME),Darwin)
	@rm -rf $(MAC_APP)
else ifeq ($(OS_NAME),Linux)
	@bash tools/install_desktop.sh uninstall $(DIR_gui)/bin/snake-gui
endif
	$(call say,$(YELLOW),Removed. Your scores were kept; make clean-user-data removes them)

# Redraws assets/icon.* with the game's own sprites (needs Python 3 and Pillow)
icon:
	$(call say,$(BLUE),Drawing the icon...)
	@$(CMAKE) -S . -B build-icon $(CMAKE_FLAGS) -DSNAKE_GAME_BUILD_ICON=ON -DSNAKE_GAME_BUILD_APPS=OFF -DSNAKE_GAME_BUILD_TESTS=OFF
	@$(CMAKE) --build build-icon --target snake_icon -j
	@python3 tools/make_icon.py --tool build-icon/bin/snake_icon
	$(call say,$(GREEN),Icons written to assets/)

# Writes every piece of the soundtrack as a WAV file in music/ (ignored by git), played by the game's own synth
music:
	$(call say,$(BLUE),Rendering the soundtrack...)
	@$(CMAKE) -S . -B build-music $(CMAKE_FLAGS) -DSNAKE_GAME_BUILD_MUSIC_TOOL=ON -DSNAKE_GAME_BUILD_APPS=OFF -DSNAKE_GAME_BUILD_TESTS=OFF
	@$(CMAKE) --build build-music --target snake_render_music -j
	@mkdir -p music && rm -f music/*.wav && build-music/bin/snake_render_music music
	$(call say,$(GREEN),WAV files written to music/)

# Integrated loudness of each file of music/; they should be within 0.3 LUFS of each other (the "gain" of each
# piece in tools/gen_score.py sets it). Needs ffmpeg
loudness:
	@for f in music/*.wav; do printf '%-60s' "$$f"; ffmpeg -nostats -i "$$f" -af ebur128=peak=true -f null - 2>&1 \
		| grep -A1 "Integrated loudness" | grep -o "I: *[-0-9.]* LUFS"; done

# Checks the mix of each piece (peak, loudness, melody against accompaniment, registers) and fails when a piece clips
# or its tune is not clearly above the accompaniment; see tools/check_balance.py. Needs numpy, scipy and ffmpeg
balance: music
	$(call say,$(BLUE),Checking the balance of the soundtrack...)
	@python3 tools/check_balance.py

# ==============================
# Docker
# ==============================

.PHONY: docker test-docker run-docker clean-docker clean-docker-data rebuild-docker

docker:
	$(call say,$(BLUE),Building the Docker image...)
	@docker build -t $(IMAGE) .
	$(call say,$(GREEN),Docker image built: $(IMAGE))

test-docker:
	$(call say,$(BLUE),Running the tests in Docker...)
	@docker build --target test -t $(IMAGE)-test .

run-docker: docker
	$(call say,$(GREEN),Running in Docker with persistent scores...)
	@docker volume create $(VOLUME) >/dev/null
	@docker run --rm -it -v $(VOLUME):/app/data $(IMAGE)

clean-docker:
	$(call say,$(YELLOW),Removing the Docker image...)
	@docker rmi -f $(IMAGE) $(IMAGE)-test 2>/dev/null || true

clean-docker-data:
	$(call say,$(YELLOW),Removing the Docker score volume...)
	@docker volume rm $(VOLUME) 2>/dev/null || true

rebuild-docker: clean-docker docker

# ==============================
# Help
# ==============================

# head TITLE: a section title; row TARGET TEXT: a target and what it does, in two columns
head = @printf '\n$(BLUE)%s$(NC)\n' '$(1)'
row = @printf '  $(GREEN)make %-18s$(NC) %s\n' '$(1)' '$(2)'

.PHONY: help
help:
	@printf '$(GREEN)Snake Game - Makefile Options$(NC)\n'
	$(call head,Build)
	$(call row,term,Terminal game: build/bin/snake - the default)
	$(call row,audio,Terminal game with music: build-audio/bin/snake)
	$(call row,gui,Graphical game: build-sdl/bin/snake-gui)
	$(call head,Run)
	$(call row,run-term,Build and play - make run is the same)
	$(call row,run-audio,Build and play with music)
	$(call row,run-gui,Build and play the graphical game)
	$(call head,Test)
	$(call row,test-term,Build and run the tests - make test is the same)
	$(call row,test-audio,Same for the audio build)
	$(call row,test-gui,Same for the graphical build)
	$(call row,test-all,All three)
	$(call head,Clean)
	$(call row,clean,Remove all build directories)
	$(call row,clean-data,Remove the local scores)
	$(call row,clean-user-data,Remove the scores kept for your user)
	$(call row,rebuild,Clean and rebuild the terminal game)
	$(call head,Tools)
	$(call row,install-gui,Install the graphical game for your user)
	$(call row,uninstall-gui,Remove it - the scores are kept)
	$(call row,icon,Redraw assets/icon.* - needs Python and Pillow)
	$(call row,music,Write the soundtrack as WAV files in music/)
	$(call row,loudness,Show the loudness of each file of music/)
	$(call row,balance,Check the mix of each piece - peak and melody against accompaniment)
	$(call head,Docker)
	$(call row,docker,Build the image)
	$(call row,test-docker,Run the tests inside the Docker build)
	$(call row,run-docker,Build and play with a persistent score volume)
	$(call row,clean-docker,Remove the image)
	$(call row,clean-docker-data,Remove the score volume)
	$(call row,rebuild-docker,Clean and rebuild the image)
