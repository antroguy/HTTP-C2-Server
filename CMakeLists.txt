cmake_minimum_required(VERSION 3.10)

project(HTTPS-C2-SERVER)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")
find_package(Threads REQUIRED)

INCLUDE_DIRECTORIES(include ${CMAKE_SOURCE_DIR}/include /usr/include)
add_library(library src/client.cpp src/server.cpp src/stegImage.cpp src/cli.cpp)

add_executable(serverMain src/serverMain.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BIN_DIR}/bin)
target_link_libraries(library libpng.a libz.a ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(serverMain library)