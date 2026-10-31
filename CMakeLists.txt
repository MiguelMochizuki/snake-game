cmake_minimum_required(VERSION 3.16)
project(snake_game VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(SNAKE_GAME_BUILD_APPS "Build the game executable" ON)
option(SNAKE_GAME_BUILD_TESTS "Build unit and end-to-end tests" ON)

# Compiler warnings, shared by all targets (including tests)
function(snake_game_set_warnings target)
	if(MSVC)
		target_compile_options(${target} PRIVATE /W4)
	else()
		target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
	endif()
endfunction()

# Everything except the executable's main() lives in the core library
add_library(snake_core STATIC
	src/game/entity.cpp
	src/game/food.cpp
	src/game/snake_segment.cpp
	src/game/snake.cpp
	src/game/game_state.cpp
	src/game/game_engine.cpp
	src/game/level.cpp
	src/input/wasd_input_handler.cpp
	src/input/arrow_input_handler.cpp
	src/input/command_input_handler.cpp
	src/input/composite_input_handler.cpp
	src/input/key_decoder.cpp
	src/input/terminal_key_source.cpp
	src/menu/menu_model.cpp
	src/menu/menu_controller.cpp
	src/menu/terminal_menu_view.cpp
	src/render/terminal_renderer.cpp
	src/render/sound.cpp
	src/score/score_manager.cpp
)
target_include_directories(snake_core PUBLIC include)
snake_game_set_warnings(snake_core)
if(UNIX)
	target_link_libraries(snake_core PUBLIC pthread)
endif()

if(SNAKE_GAME_BUILD_APPS)
	add_subdirectory(apps)
endif()

if(SNAKE_GAME_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
