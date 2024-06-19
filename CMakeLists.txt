cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

if(!EMSCPRITEN)
  set(CMAKE_CXX_FLAGS "-g -O0")
endif()

project(paranoixa)
include(webgpu.cmake)

if(WIN32)
  include_directories(
    ${CMAKE_SOURCE_DIR}/include

    ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-wgpu-src/include
  )
endif()

if(EMSCRIPTEN)
  include_directories(
    ${CMAKE_SOURCE_DIR}/include
  )
endif()

file(GLOB
  SOURCE_FILES
  src/paranoixa.cpp
  src/*/*.cpp
  src/*/*/*.cpp
)
add_library(paranoixa STATIC ${SOURCE_FILES})

add_subdirectory(test)
