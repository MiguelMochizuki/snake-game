# Snake Game Makefile
# Author: Miguel Mochizuki Silva
# Description: Thin wrapper around CMake and Docker: build, run, test and clean

BUILD ?= build
DATA_DIR = data

DOCKER_IMAGE = snake-game
DOCKER_VOLUME = snake-data

.PHONY: all run test clean clean-data docker run-docker test-docker clean-docker clean-docker-data help

all:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD) -j

run: all
	$(BUILD)/bin/snake

test: all
	ctest --test-dir $(BUILD) --output-on-failure

clean:
	rm -rf $(BUILD)

clean-data:
	rm -rf $(DATA_DIR)

docker:
	docker build -t $(DOCKER_IMAGE) .

test-docker:
	docker build --target test -t $(DOCKER_IMAGE)-test .

run-docker: docker
	docker volume create $(DOCKER_VOLUME) >/dev/null
	docker run --rm -it -v $(DOCKER_VOLUME):/app/data $(DOCKER_IMAGE)

clean-docker:
	docker rmi -f $(DOCKER_IMAGE) $(DOCKER_IMAGE)-test 2>/dev/null || true

clean-docker-data:
	docker volume rm $(DOCKER_VOLUME) 2>/dev/null || true

help:
	@echo "make            build (Release) into $(BUILD)/"
	@echo "make run        build and play"
	@echo "make test       build, then run unit and e2e tests"
	@echo "make clean      remove $(BUILD)/"
	@echo "make clean-data remove local score data"
	@echo "make docker | run-docker | test-docker | clean-docker | clean-docker-data"
