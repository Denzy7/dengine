# glue code


set(DENGINE_INIT_CONF_VERSION "0.7.2")
configure_file(src/dengine.ini.in dengine.ini)
configure_file(src/dengine.ini.h.in dengine.ini.h)
dengine_rc_compile(
    ${CMAKE_CURRENT_BINARY_DIR}/dengine.ini
    ${CMAKE_CURRENT_BINARY_DIR}/dengine.ini.c
    "-enb"
    )
add_library(dengine_glue_lib STATIC
    src/dengine.c
    ${CMAKE_CURRENT_BINARY_DIR}/dengine.ini.c
    )
set_target_properties(dengine_glue_lib PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
target_include_directories(dengine_glue_lib PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(dengine_glue_lib PRIVATE dengine-core dengine-utils dengine-gui dengine-script)

install(TARGETS dengine_glue_lib DESTINATION lib)
