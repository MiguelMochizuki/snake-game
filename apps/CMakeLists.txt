add_executable(snake snake.cpp)
target_link_libraries(snake PRIVATE snake_core)
snake_game_set_warnings(snake)
