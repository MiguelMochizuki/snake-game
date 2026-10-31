include(FetchContent)

FetchContent_Declare(googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
	URL_HASH SHA256=7b42b4d6ed48810c5362c265a17faebe90dc2373c885e5216439d37927f02926
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Unit tests: every tests/unit/*.cpp is picked up automatically
file(GLOB UNIT_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp)
if(WIN32)
	list(REMOVE_ITEM UNIT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit/terminal_key_source_test.cpp)
endif()

if(NOT SNAKE_GAME_BUILD_AUDIO)
	list(REMOVE_ITEM UNIT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit/audio_device_test.cpp)
endif()

set(GUI_TESTS canvas_test bitmap_font_test sdl_keys_test sdl_renderer_test sdl_menu_view_test demo_pilot_test)
if(NOT SNAKE_GAME_BUILD_GUI)
	foreach(name ${GUI_TESTS})
		list(REMOVE_ITEM UNIT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit/${name}.cpp)
	endforeach()
endif()

add_executable(snake_unit_tests ${UNIT_SOURCES})
target_link_libraries(snake_unit_tests PRIVATE snake_core GTest::gtest_main)
if(SNAKE_GAME_BUILD_GUI)
	target_link_libraries(snake_unit_tests PRIVATE snake_gui_core)
endif()
target_compile_definitions(snake_unit_tests PRIVATE
	SNAKE_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden")
snake_game_set_warnings(snake_unit_tests)

include(GoogleTest)
gtest_discover_tests(snake_unit_tests PROPERTIES LABELS unit TIMEOUT 30)

# End-to-end tests (Linux only): every tests/e2e/test_*.py is registered automatically
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND TARGET snake)
	find_package(Python3 COMPONENTS Interpreter)
	if(Python3_Interpreter_FOUND)
		# Default from the configure-time environment so `SNAKE_E2E_TIMEOUT_SCALE=3 cmake ...` works
		if(DEFINED ENV{SNAKE_E2E_TIMEOUT_SCALE})
			set(E2E_SCALE_DEFAULT $ENV{SNAKE_E2E_TIMEOUT_SCALE})
		else()
			set(E2E_SCALE_DEFAULT 1)
		endif()
		set(SNAKE_E2E_TIMEOUT_SCALE ${E2E_SCALE_DEFAULT} CACHE STRING "Multiplier for every e2e timeout (slow or emulated hosts)")
		math(EXPR E2E_TIMEOUT "60 * ${SNAKE_E2E_TIMEOUT_SCALE}")
		file(GLOB E2E_SCRIPTS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/e2e/test_*.py)
		list(FILTER E2E_SCRIPTS EXCLUDE REGEX "/test_gui_[^/]*\\.py$")
		foreach(script ${E2E_SCRIPTS})
			get_filename_component(stem ${script} NAME_WE)
			string(REGEX REPLACE "^test_" "" name ${stem})
			add_test(NAME e2e_${name} COMMAND Python3::Interpreter -B ${script} $<TARGET_FILE:snake>)
			set_tests_properties(e2e_${name} PROPERTIES
				LABELS e2e
				TIMEOUT ${E2E_TIMEOUT}
				ENVIRONMENT "SNAKE_E2E_TIMEOUT_SCALE=${SNAKE_E2E_TIMEOUT_SCALE};SNAKE_AUDIO=off")
		endforeach()
		# GUI end-to-end tests run snake-gui instead of snake
		if(SNAKE_GAME_BUILD_GUI)
			file(GLOB GUI_E2E_SCRIPTS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/e2e/test_gui_*.py)
			foreach(script ${GUI_E2E_SCRIPTS})
				get_filename_component(stem ${script} NAME_WE)
				string(REGEX REPLACE "^test_" "" name ${stem})
				add_test(NAME e2e_${name} COMMAND Python3::Interpreter -B ${script} $<TARGET_FILE:snake-gui>)
				set_tests_properties(e2e_${name} PROPERTIES LABELS e2e TIMEOUT ${E2E_TIMEOUT})
			endforeach()
		endif()
	else()
		message(STATUS "Python3 not found: e2e tests not registered")
	endif()
endif()
