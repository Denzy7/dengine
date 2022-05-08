add_executable(python-dengine-script-mingw32 python-mingw32.c)
target_link_libraries(python-dengine-script-mingw32 ${dengine} ${python3_LIBRARIES})
