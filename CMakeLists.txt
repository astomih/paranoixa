cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

project(paranoixa)
include(webgpu.cmake)
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-emscripten-src/include
  ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-wgpu-src/include
  ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-dawn-src/include
)
file(GLOB
  SOURCE_FILES
  src/paranoixa.cpp
  src/*/*.cpp
  src/*/*/*.cpp
)
add_library(paranoixa STATIC ${SOURCE_FILES})

add_subdirectory(test)