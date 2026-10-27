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

add_executable(snake_unit_tests ${UNIT_SOURCES})
target_link_libraries(snake_unit_tests PRIVATE snake_core GTest::gtest_main)
target_compile_definitions(snake_unit_tests PRIVATE
	SNAKE_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden")
snake_set_warnings(snake_unit_tests)

include(GoogleTest)
gtest_discover_tests(snake_unit_tests PROPERTIES LABELS unit TIMEOUT 30)

# End-to-end tests (Linux only): every tests/e2e/test_*.py is registered automatically
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
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
		foreach(script ${E2E_SCRIPTS})
			get_filename_component(stem ${script} NAME_WE)
			string(REGEX REPLACE "^test_" "" name ${stem})
			add_test(NAME e2e_${name} COMMAND Python3::Interpreter -B ${script} $<TARGET_FILE:snake>)
			set_tests_properties(e2e_${name} PROPERTIES
				LABELS e2e
				TIMEOUT ${E2E_TIMEOUT}
				ENVIRONMENT SNAKE_E2E_TIMEOUT_SCALE=${SNAKE_E2E_TIMEOUT_SCALE})
		endforeach()
	else()
		message(STATUS "Python3 not found: e2e tests not registered")
	endif()
endif()
