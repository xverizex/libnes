cmake_minimum_required(VERSION 3.16)

project(nes LANGUAGES C)

add_library(nes
    cpunes.h cpunes.c
    instr.c palette.c 
    sdl3_opengl4.c sdl3_opengl4.h
    controller.h
    linux/platform.c)

target_include_directories(nes PRIVATE
    "."
    "glad/include"
)

include(GNUInstallDirs)
install(TARGETS nes
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
