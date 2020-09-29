cmake_minimum_required(VERSION 3.10)

project(HTTPS-C2-SERVER)

INCLUDE_DIRECTORIES(include ${CMAKE_SOURCE_DIR}/include)
add_library(library src/client.cpp src/server.cpp)

add_executable(serverMain src/serverMain.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BIN_DIR}/bin)
target_link_libraries(serverMain library)