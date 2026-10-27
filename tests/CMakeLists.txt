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
	list(REMOVE_ITEM UNIT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit/wasd_input_handler_test.cpp)
endif()

add_executable(snake_unit_tests ${UNIT_SOURCES})
target_link_libraries(snake_unit_tests PRIVATE snake_core GTest::gtest_main)
target_compile_definitions(snake_unit_tests PRIVATE
	SNAKE_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden")
snake_set_warnings(snake_unit_tests)

include(GoogleTest)
gtest_discover_tests(snake_unit_tests PROPERTIES LABELS unit)
