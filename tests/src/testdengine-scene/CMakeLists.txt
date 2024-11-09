# test ecs
add_executable(testdengine-scene-ecs
        scene-ecs.c)
target_link_libraries(testdengine-scene-ecs
        dengine-core dengine-script dengine-utils dengine-gui dengine-scene dengine-model dengine_glue_lib)
set_property(TARGET testdengine-scene-ecs PROPERTY C_STANDARD 99)
if(MINGW)
    set(exe testdengine-scene-ecs)
    set(ext exe)
    set(cwd ${PROJECT_SOURCE_DIR}/deps)
    configure_file(${PROJECT_SOURCE_DIR}/tools/mingw/mingw_copylibs_install.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/testdengine-scene-ecs_install.cmake @ONLY)
    install(SCRIPT  ${CMAKE_CURRENT_BINARY_DIR}/testdengine-scene-ecs_install.cmake)
endif()
