cmake_minimum_required(VERSION 3.24)

project(
    libremiditest
    VERSION 1.0.0
    DESCRIPTION "libremiditest"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libremidi CONFIG REQUIRED)

add_executable(libremiditest src/main.cpp)

target_link_libraries(libremiditest PRIVATE libremidi)

target_link_directories(libremiditest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)