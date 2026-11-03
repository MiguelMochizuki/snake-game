# Snake Game Makefile
# Author: Miguel Mochizuki Silva
# Description: Makefile to orchestrate CMake and Docker build processes

# ==============================
# Configuration
# ==============================

# Three build variants, each in its own directory so they never clash:
#   terminal  build/        the terminal game, no extra dependencies (BUILD=dir overrides)
#   audio     build-audio/  the terminal game with chord music (fetches miniaudio)
#   gui       build-sdl/    the graphical game, implies audio (fetches SDL3 and miniaudio)
BUILD ?= build
BUILD_DIR = $(BUILD)
AUDIO_BUILD_DIR = build-audio
GUI_BUILD_DIR = build-sdl
DATA_DIR = data

# CMake options
CMAKE = cmake
CMAKE_BUILD_TYPE ?= Release
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# Executables
EXECUTABLE = $(BUILD_DIR)/bin/snake
AUDIO_EXECUTABLE = $(AUDIO_BUILD_DIR)/bin/snake
GUI_EXECUTABLE = $(GUI_BUILD_DIR)/bin/snake-gui

# Docker options
DOCKER_IMAGE = snake-game
DOCKER_CONTAINER = snake-game-container
DOCKER_VOLUME = snake-data

# Docker cache tracking
DOCKER_CACHE = .docker-build-cache

# Files that trigger a rebuild (build trees and tooling folders are skipped)
SOURCES = $(shell find . \( -path './build*' -o -path ./.git -o -path ./.superpowers \) -prune -o \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' \) -print 2>/dev/null)
TEST_FILES = $(shell find tests \( -name '*.py' -o -name '*.txt' \) -print 2>/dev/null)

# Colors
GREEN = \033[0;32m
BLUE = \033[0;34m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m

# Configures and builds one variant. Arguments: label, build directory, extra CMake flags, executable
define build_variant
	@echo "$(BLUE)Configuring CMake ($(1))...$(NC)"
	@$(CMAKE) -S . -B $(2) $(CMAKE_FLAGS) $(3)
	@echo "$(BLUE)Building project...$(NC)"
	@$(CMAKE) --build $(2) -j
	@echo "$(GREEN)Build complete!$(NC)"
	@echo "$(GREEN)Executable: $(4)$(NC)"
	@touch $(2)/.built
endef

# Runs the tests of one variant. Arguments: label, build directory
define test_variant
	@echo "$(BLUE)Running tests ($(1))...$(NC)"
	@ctest --test-dir $(2) --output-on-failure
endef

# ==============================
# Build targets
# ==============================

.PHONY: all local audio gui
all local: $(BUILD_DIR)/.built
audio: $(AUDIO_BUILD_DIR)/.built
gui: $(GUI_BUILD_DIR)/.built

# Each variant rebuilds only if source files, tests or CMakeLists.txt changed. The .built stamp is
# written only after the whole build succeeded, so a failed build is never mistaken for a finished one
$(BUILD_DIR)/.built: $(SOURCES) $(TEST_FILES) CMakeLists.txt
	$(call build_variant,terminal,$(BUILD_DIR),,$(EXECUTABLE))

$(AUDIO_BUILD_DIR)/.built: $(SOURCES) $(TEST_FILES) CMakeLists.txt
	$(call build_variant,terminal with audio,$(AUDIO_BUILD_DIR),-DSNAKE_GAME_BUILD_AUDIO=ON,$(AUDIO_EXECUTABLE))

$(GUI_BUILD_DIR)/.built: $(SOURCES) $(TEST_FILES) CMakeLists.txt
	$(call build_variant,graphical,$(GUI_BUILD_DIR),-DSNAKE_GAME_BUILD_GUI=ON,$(GUI_EXECUTABLE))

# ==============================
# Run targets
# ==============================

.PHONY: run run-local run-audio run-gui
run run-local: all
	@echo "$(GREEN)Running Snake Game...$(NC)"
	@$(EXECUTABLE)

run-audio: audio
	@echo "$(GREEN)Running Snake Game with music...$(NC)"
	@$(AUDIO_EXECUTABLE)

run-gui: gui
	@echo "$(GREEN)Running Snake Game (graphical)...$(NC)"
	@$(GUI_EXECUTABLE)

# ==============================
# Test targets
# ==============================

.PHONY: test test-local test-audio test-gui test-all
test test-local: all
	$(call test_variant,terminal,$(BUILD_DIR))

test-audio: audio
	$(call test_variant,audio,$(AUDIO_BUILD_DIR))

test-gui: gui
	$(call test_variant,graphical,$(GUI_BUILD_DIR))

test-all: test test-audio test-gui

# ==============================
# Clean targets
# ==============================

.PHONY: clean clean-local clean-data clean-local-data rebuild rebuild-local
clean clean-local:
	@echo "$(YELLOW)Cleaning build files...$(NC)"
	@rm -rf $(BUILD_DIR) $(AUDIO_BUILD_DIR) $(GUI_BUILD_DIR)
	@echo "$(YELLOW)Clean complete!$(NC)"

clean-data clean-local-data:
	@echo "$(YELLOW)Removing local data...$(NC)"
	@rm -rf $(DATA_DIR)
	@echo "$(YELLOW)Local data removed!$(NC)"

rebuild rebuild-local: clean all

# ==============================
# Tools
# ==============================

# Gives the graphical game its icon and an app-menu entry for the current user (Linux desktops)
.PHONY: install-gui uninstall-gui
install-gui: gui
	@bash tools/install_desktop.sh install $(GUI_EXECUTABLE)
	@echo "$(GREEN)Installed the Snake entry and icon for $(USER); log out and in if your dock does not pick it up$(NC)"

uninstall-gui:
	@bash tools/install_desktop.sh uninstall $(GUI_EXECUTABLE)
	@echo "$(YELLOW)Removed the Snake entry and icon$(NC)"

# Needs Python 3 with Pillow (pip install pillow)
.PHONY: icon
icon:
	@echo "$(BLUE)Drawing the app icon...$(NC)"
	@python3 tools/make_icon.py
	@echo "$(GREEN)Icons written to assets/$(NC)"

# ==============================
# Docker targets
# ==============================

# Build Docker image only if Dockerfile or source files changed
$(DOCKER_CACHE): Dockerfile .gitattributes $(SOURCES) $(TEST_FILES) CMakeLists.txt
	@echo "$(BLUE)Building Docker image...$(NC)"
	@docker build -t $(DOCKER_IMAGE) .
	@touch $(DOCKER_CACHE)
	@echo "$(GREEN)Docker image built: $(DOCKER_IMAGE)$(NC)"

.PHONY: docker
docker: $(DOCKER_CACHE)

.PHONY: test-docker
test-docker:
	@echo "$(BLUE)Running tests in Docker...$(NC)"
	@docker build --target test -t $(DOCKER_IMAGE)-test .

.PHONY: run-docker
run-docker: docker
	@echo "$(GREEN)Running Snake Game in Docker with persistent scores...$(NC)"
	@docker volume create $(DOCKER_VOLUME) 2>/dev/null || true
	@docker run --rm -it \
		-v $(DOCKER_VOLUME):/app/data \
		--name $(DOCKER_CONTAINER) \
		$(DOCKER_IMAGE)

.PHONY: clean-docker
clean-docker:
	@echo "$(YELLOW)Cleaning Docker...$(NC)"
	@docker rm -f $(DOCKER_CONTAINER) 2>/dev/null || true
	@docker rmi -f $(DOCKER_IMAGE) $(DOCKER_IMAGE)-test 2>/dev/null || true
	@rm -f $(DOCKER_CACHE)
	@echo "$(YELLOW)Docker clean complete!$(NC)"

.PHONY: clean-docker-data
clean-docker-data:
	@echo "$(YELLOW)Removing Docker data volume...$(NC)"
	@docker volume rm $(DOCKER_VOLUME) 2>/dev/null || true
	@echo "$(YELLOW)Docker data volume removed!$(NC)"

.PHONY: rebuild-docker
rebuild-docker: clean-docker docker

# ==============================
# Help
# ==============================

.PHONY: help
help:
	@echo "$(GREEN)Snake Game - Makefile Options$(NC)"
	@echo ""
	@echo "$(BLUE)Build (only if changed):$(NC)"
	@echo "  make                - Terminal game: $(EXECUTABLE)"
	@echo "  make audio          - Terminal game with music: $(AUDIO_EXECUTABLE)"
	@echo "  make gui            - Graphical game: $(GUI_EXECUTABLE)"
	@echo ""
	@echo "$(BLUE)Run:$(NC)"
	@echo "  make run            - Build and run the terminal game"
	@echo "  make run-audio      - Build and run the terminal game with music"
	@echo "  make run-gui        - Build and run the graphical game"
	@echo ""
	@echo "$(BLUE)Test:$(NC)"
	@echo "  make test           - Build and run the unit and e2e tests (terminal)"
	@echo "  make test-audio     - Same, for the audio build"
	@echo "  make test-gui       - Same, for the graphical build"
	@echo "  make test-all       - All three"
	@echo ""
	@echo "$(BLUE)Clean:$(NC)"
	@echo "  make clean          - Remove all build directories"
	@echo "  make clean-data     - Remove local score data"
	@echo "  make rebuild        - Clean and rebuild the terminal game"
	@echo ""
	@echo "$(BLUE)Tools:$(NC)"
	@echo "  make install-gui    - Add the Snake icon and app-menu entry for this user (Linux)"
	@echo "  make uninstall-gui  - Remove them"
	@echo "  make icon           - Redraw assets/icon.* (needs Python and Pillow)"
	@echo ""
	@echo "$(BLUE)Docker:$(NC)"
	@echo "  make docker         - Build Docker image (only if changed)"
	@echo "  make test-docker    - Run the test suite in Docker (test stage only)"
	@echo "  make run-docker     - Build and run in Docker (only if changed)"
	@echo "  make clean-docker   - Remove Docker image, container and cache"
	@echo "  make clean-docker-data - Remove Docker data volume"
	@echo "  make rebuild-docker - Clean and rebuild Docker image"
	@echo ""
	@echo "The old *-local names (local, run-local, test-local, ...) still work."
