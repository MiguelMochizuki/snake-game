cmake_minimum_required(VERSION 3.14)

project(SnakeGame VERSION 1.0.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files (everything except main.cpp goes in the core library)
set(CORE_SOURCES
	src/entity.cpp
	src/food.cpp
	src/snake_segment.cpp
	src/snake.cpp
	src/game_state.cpp
	src/wasd_input_handler.cpp
	src/terminal_renderer.cpp
	src/game_engine.cpp
	src/score_manager.cpp
	src/arrow_input_handler.cpp
	src/command_input_handler.cpp
	src/composite_input_handler.cpp
	src/key_decoder.cpp
	src/level.cpp
	src/sound.cpp
	src/menu_model.cpp
	src/terminal_menu_view.cpp
	src/menu_controller.cpp
	src/terminal_key_source.cpp
)

# Compiler warnings, shared by all targets (including tests)
function(snake_set_warnings target)
	if(MSVC)
		target_compile_options(${target} PRIVATE /W4)
	else()
		target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
	endif()
endfunction()

# Core library
add_library(snake_core STATIC ${CORE_SOURCES})
target_include_directories(snake_core PUBLIC include)
snake_set_warnings(snake_core)
if(UNIX)
	target_link_libraries(snake_core PUBLIC pthread)
endif()

# Executable
add_executable(snake src/main.cpp)
target_link_libraries(snake PRIVATE snake_core)
snake_set_warnings(snake)

# Tests
include(CTest)
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
