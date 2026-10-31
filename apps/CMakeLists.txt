add_executable(snake snake.cpp)
target_link_libraries(snake PRIVATE snake_core)
snake_game_set_warnings(snake)

if(SNAKE_GAME_BUILD_GUI)
	add_executable(snake-gui snake_gui.cpp)
	target_link_libraries(snake-gui PRIVATE snake_gui_core)
	snake_game_set_warnings(snake-gui)
endif()
