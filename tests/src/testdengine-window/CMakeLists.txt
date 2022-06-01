#basic window
add_executable(testdengine-window-basic
        window-basic.c)
target_link_libraries(testdengine-window-basic
        dengine-core dengine-utils)
set_property(TARGET testdengine-window-basic PROPERTY C_STANDARD 99)

#window with glClearColor
add_executable(testdengine-window-colored
        window-colored.c
        )
target_link_libraries(testdengine-window-colored
        dengine-core dengine-utils)
set_property(TARGET testdengine-window-colored PROPERTY C_STANDARD 99)

#windows sharing a texture object (gui fontmap)
add_executable(testdengine-window-shared
        window-shared.c
        )
target_link_libraries(testdengine-window-shared
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-window-shared PROPERTY C_STANDARD 99)
