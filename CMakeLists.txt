cmake_minimum_required(VERSION 3.16)

project(libnes LANGUAGES C)

add_library(libnes
    cpunes.h cpunes.c
    handlers.c palette.c 
    sdl3_opengl4.c sdl3_opengl4.h
    controller.h
    glad/src/gl.c
    linux/platform.c)

target_include_directories(libnes PRIVATE
    "."
)

include(GNUInstallDirs)
install(TARGETS libnes
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
