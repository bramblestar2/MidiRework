cmake_minimum_required(VERSION 3.24)

project(
    libremiditest
    VERSION 1.0.0
    DESCRIPTION "libremiditest"
    LANGUAGES CXX
)

if (LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JACK REQUIRED jack)
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libremidi)

find_package(spdlog CONFIG REQUIRED)

add_executable(libremiditest 
    src/main.cpp

    include/Midi/MidiDevice.h src/Midi/MidiDevice.cpp
    include/Midi/MidiManager.h src/Midi/MidiManager.cpp
    include/Midi/types.h
)

target_link_libraries(libremiditest PRIVATE 
    libremidi
    spdlog::spdlog
)

if (LINUX)
    target_link_libraries(libremiditest PRIVATE ${JACK_LIBRARIES})
endif()

target_include_directories(libremiditest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include(GNUInstallDirs)
install(TARGETS libremiditest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)