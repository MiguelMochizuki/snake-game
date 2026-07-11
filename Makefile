# Snake Game Makefile
# Author: Miguel Mochizuki Silva
# Description: Makefile to orchestrate CMake and Docker build processes

# Directories
BUILD_DIR = build

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

# Colors
GREEN = \033[0;32m
BLUE = \033[0;34m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m

# ==============================
# Local targets
# ==============================

.PHONY: local
local: $(EXECUTABLE)

# Build only if source files or CMakeLists.txt changed
$(EXECUTABLE): $(shell find . -name "*.cpp" -o -name "*.h" -o -name "*.c" 2>/dev/null) CMakeLists.txt
	@echo "$(BLUE)Configuring CMake...$(NC)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) ..
	@echo "$(BLUE)Building project...$(NC)"
	@$(MAKE) -C $(BUILD_DIR)
	@echo "$(GREEN)Build complete!$(NC)"
	@echo "$(GREEN)Executable: $(EXECUTABLE)$(NC)"

.PHONY: run-local
run-local: local
	@echo "$(GREEN)Running Snake Game...$(NC)"
	@$(EXECUTABLE)

.PHONY: clean-local
clean-local:
	@echo "$(YELLOW)Cleaning build files...$(NC)"
	@rm -rf $(BUILD_DIR)
	@echo "$(YELLOW)Clean complete!$(NC)"

.PHONY: rebuild-local
rebuild-local: clean-local local

# ==============================
# Docker targets
# ==============================

# Build Docker image only if Dockerfile or source files changed
$(DOCKER_CACHE): Dockerfile $(shell find . -name "*.cpp" -o -name "*.h" -o -name "*.c" 2>/dev/null) CMakeLists.txt
	@echo "$(BLUE)Building Docker image...$(NC)"
	@docker build -t $(DOCKER_IMAGE) .
	@touch $(DOCKER_CACHE)
	@echo "$(GREEN)Docker image built: $(DOCKER_IMAGE)$(NC)"

.PHONY: docker
docker: $(DOCKER_CACHE)

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
	@docker rmi -f $(DOCKER_IMAGE) 2>/dev/null || true
	@rm -f $(DOCKER_CACHE)
	@echo "$(YELLOW)Docker clean complete!$(NC)"

.PHONY: clean-volume
clean-volume:
	@echo "$(YELLOW)Removing volume...$(NC)"
	@docker volume rm $(DOCKER_VOLUME) 2>/dev/null || true
	@echo "$(YELLOW)Volume removed!$(NC)"

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
	@echo "  make local          - Build the game locally (only if changed)"
	@echo "  make run-local      - Build and run the game locally (only if changed)"
	@echo "  make clean-local    - Remove build files"
	@echo "  make rebuild-local  - Clean and rebuild locally"
	@echo ""
	@echo "$(BLUE)Docker targets:$(NC)"
	@echo "  make docker         - Build Docker image (only if changed)"
	@echo "  make run-docker     - Build and run in Docker (only if changed)"
	@echo "  make clean-docker   - Remove Docker image, container and cache"
	@echo "  make clean-volume   - Remove the data volume"
	@echo "  make rebuild-docker - Clean and rebuild Docker image"
