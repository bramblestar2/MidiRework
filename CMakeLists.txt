cmake_minimum_required(VERSION 3.24)

project(
    libremiditest
    VERSION 1.0.0
    DESCRIPTION "libremiditest"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if (LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JACK REQUIRED jack)
endif()


FetchContent_Declare(
  readerwriterqueue
  GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(readerwriterqueue)

find_package(spdlog CONFIG REQUIRED)

add_subdirectory(libremidi)

add_library(MidiReworkCore 

    include/Midi/MidiDevice.h src/Midi/MidiDevice.cpp
    include/Midi/MidiManager.h src/Midi/MidiManager.cpp
    include/Midi/types.h
)

target_link_libraries(MidiReworkCore PUBLIC 
    libremidi
    spdlog::spdlog
)

if (LINUX)
    target_link_libraries(MidiReworkCore PUBLIC ${JACK_LIBRARIES})
endif()

target_include_directories(MidiReworkCore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


add_executable(libremidiexecutable src/main.cpp)

target_link_libraries(libremidiexecutable PRIVATE 
    MidiReworkCore
)

target_include_directories(libremidiexecutable PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


include(GNUInstallDirs)
install(TARGETS MidiReworkCore
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)