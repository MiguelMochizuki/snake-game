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
option(SNAKE_GAME_BUILD_GUI "Build snake-gui, the SDL3 front-end (fetched at configure time; implies audio)" OFF)
option(SNAKE_GAME_BUILD_AUDIO "Build with music and sound output through miniaudio (fetched at configure time)" OFF)

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
	src/audio/synth.cpp
	src/render/sound.cpp
	src/score/score_manager.cpp
)
target_include_directories(snake_core PUBLIC include)
snake_game_set_warnings(snake_core)
if(UNIX)
	target_link_libraries(snake_core PUBLIC pthread)
endif()

if(SNAKE_GAME_BUILD_GUI)
	set(SNAKE_GAME_BUILD_AUDIO ON CACHE BOOL "" FORCE)
endif()

if(SNAKE_GAME_BUILD_AUDIO)
	include(FetchContent)
	# Only the single header is needed, so skip miniaudio's own CMake project
	FetchContent_Declare(miniaudio
		URL https://github.com/mackron/miniaudio/archive/refs/tags/0.11.26.tar.gz
		URL_HASH SHA256=838ba55a21c9142adf7b76bdc4351e5cd8b7ae128f46b25ec25f94ae9969c600
		SOURCE_SUBDIR no-cmake-project
	)
	FetchContent_MakeAvailable(miniaudio)
	target_sources(snake_core PRIVATE src/audio/audio_device.cpp)
	target_include_directories(snake_core SYSTEM PRIVATE ${miniaudio_SOURCE_DIR})
	target_compile_definitions(snake_core PUBLIC SNAKE_HAVE_AUDIO)
	target_link_libraries(snake_core PUBLIC ${CMAKE_DL_LIBS})
endif()

if(SNAKE_GAME_BUILD_GUI)
	include(FetchContent)
	set(SDL_SHARED OFF CACHE BOOL "" FORCE)
	set(SDL_STATIC ON CACHE BOOL "" FORCE)
	set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
	set(SDL_TESTS OFF CACHE BOOL "" FORCE)
	set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
	FetchContent_Declare(SDL3
		URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.18.tar.gz
		URL_HASH SHA256=c4b08b950bd29d83caae0ab8d884298d27046a9d30700fb42841c50992665c90
	)
	FetchContent_MakeAvailable(SDL3)
	add_library(snake_gui_core STATIC
		src/gui/canvas.cpp
		src/gui/bitmap_font.cpp
		src/gui/sdl_app.cpp
		src/gui/sdl_key_source.cpp
		src/gui/sdl_renderer.cpp
	)
	target_include_directories(snake_gui_core PUBLIC include)
	target_link_libraries(snake_gui_core PUBLIC snake_core SDL3::SDL3)
	snake_game_set_warnings(snake_gui_core)
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
