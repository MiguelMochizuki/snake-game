# Snake Game Makefile
# Author: Miguel Mochizuki Silva
# Description: Makefile to orchestrate CMake and Docker build processes

# Directories
BUILD ?= build
BUILD_DIR = $(BUILD)
DATA_DIR = data

# CMake options
CMAKE = cmake
CMAKE_BUILD_TYPE ?= Release
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# Docker options
DOCKER_IMAGE = snake-game
DOCKER_CONTAINER = snake-game-container
DOCKER_VOLUME = snake-data

# Executable path
EXECUTABLE = $(BUILD_DIR)/bin/snake

# Docker cache tracking
DOCKER_CACHE = .docker-build-cache

# Source files that trigger a rebuild (build trees and tooling folders are skipped)
SOURCES = $(shell find . \( -path './build*' -o -path ./.git -o -path ./.superpowers \) -prune -o \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' \) -print 2>/dev/null)
TEST_FILES = $(shell find tests \( -name '*.py' -o -name '*.txt' \) -print 2>/dev/null)

# Colors
GREEN = \033[0;32m
BLUE = \033[0;34m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m

# ==============================
# Local targets
# ==============================

.PHONY: all local
all local: $(EXECUTABLE)

# Build only if source files or CMakeLists.txt changed
$(EXECUTABLE): $(SOURCES) CMakeLists.txt
	@echo "$(BLUE)Configuring CMake...$(NC)"
	@$(CMAKE) -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)
	@echo "$(BLUE)Building project...$(NC)"
	@$(CMAKE) --build $(BUILD_DIR) -j
	@echo "$(GREEN)Build complete!$(NC)"
	@echo "$(GREEN)Executable: $(EXECUTABLE)$(NC)"

.PHONY: run run-local
run run-local: all
	@echo "$(GREEN)Running Snake Game...$(NC)"
	@$(EXECUTABLE)

.PHONY: test test-local
test test-local: all
	@echo "$(BLUE)Running tests...$(NC)"
	@ctest --test-dir $(BUILD_DIR) --output-on-failure

.PHONY: clean clean-local
clean clean-local:
	@echo "$(YELLOW)Cleaning build files...$(NC)"
	@rm -rf $(BUILD_DIR)
	@echo "$(YELLOW)Clean complete!$(NC)"

.PHONY: clean-data clean-local-data
clean-data clean-local-data:
	@echo "$(YELLOW)Removing local data...$(NC)"
	@rm -rf $(DATA_DIR)
	@echo "$(YELLOW)Local data removed!$(NC)"

.PHONY: rebuild rebuild-local
rebuild rebuild-local: clean all

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
	@echo "$(BLUE)Local targets:$(NC)"
	@echo "  make                - Build the game (only if changed): $(EXECUTABLE)"
	@echo "  make run            - Build and run the game"
	@echo "  make test           - Build, then run unit and e2e tests"
	@echo "  make clean          - Remove build files"
	@echo "  make clean-data     - Remove local data files"
	@echo "  make rebuild        - Clean and rebuild"
	@echo "  (the old *-local names still work: local, run-local, test-local, ...)"
	@echo ""
	@echo "$(BLUE)Docker targets:$(NC)"
	@echo "  make docker         - Build Docker image (only if changed)"
	@echo "  make test-docker    - Run the test suite in Docker (test stage only)"
	@echo "  make run-docker     - Build and run in Docker (only if changed)"
	@echo "  make clean-docker   - Remove Docker image, container and cache"
	@echo "  make clean-docker-data - Remove Docker data volume"
	@echo "  make rebuild-docker - Clean and rebuild Docker image"
