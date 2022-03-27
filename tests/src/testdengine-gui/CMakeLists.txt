#text w/ window coordinates
add_executable(testdengine-gui-textwindow
        textwindow.c
        )
target_link_libraries(testdengine-gui-textwindow
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-gui-textwindow PROPERTY C_STANDARD 99)

#simple textured panels + some text
add_executable(testdengine-gui-panel
        panel.c
        )
target_link_libraries(testdengine-gui-panel
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-gui-panel PROPERTY C_STANDARD 99)

#its a panel with text and input checks
add_executable(testdengine-gui-button
        button.c
        )
target_link_libraries(testdengine-gui-button
        dengine-core dengine-utils dengine-gui)
set_property(TARGET testdengine-gui-button PROPERTY C_STANDARD 99)
