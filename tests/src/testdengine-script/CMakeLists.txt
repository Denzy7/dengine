add_executable(testdengine-script-python script-python.c)
target_link_libraries(testdengine-script-python dengine dengine-script)
set_property(TARGET testdengine-script-python PROPERTY C_STANDARD 99)
