cmake_minimum_required(VERSION 3.16)

project(nes_emu LANGUAGES C)



add_executable(nes_emu
    cpunes.h cpunes.c
    handlers.c)

target_include_directories(nes_emu PRIVATE
    "."
)

include(GNUInstallDirs)
install(TARGETS nes_emu
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
