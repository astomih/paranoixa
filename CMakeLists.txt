cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 20)

project(paranoixa)
add_subdirectory(test)
include(webgpu.cmake)
add_subdirectory(library/SDL EXCLUDE_FROM_ALL)
find_package(Vulkan)

if(WIN32)
  set(CMAKE_CXX_FLAGS "-g -O0")
  set(PARANOIXA_INCLUDE_DIRS
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/library/SDL/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-wgpu-src/include
    ${CMAKE_SOURCE_DIR}/library/volk
  )
endif()

if(EMSCRIPTEN)
  include_directories(
    ${CMAKE_SOURCE_DIR}/include
  )
endif()

file(GLOB
  SOURCE_FILES
  source/paranoixa.cpp
  source/*/*.cpp
)

if(EMSCRIPTEN)
  file(GLOB
    SOURCE_FILES
    ${SOURCE_FILES}
    source/renderer/webgpu/*.cpp
  )
endif()

if(WIN32)
  file(GLOB
    SOURCE_FILES
    ${SOURCE_FILES}
    library/volk/volk.c
    source/*/*/*.cpp
  )
endif()

find_library(Vulkan SDL3)
add_library(paranoixa STATIC ${SOURCE_FILES})
target_include_directories(paranoixa PUBLIC ${PARANOIXA_INCLUDE_DIRS})
target_link_libraries(paranoixa PUBLIC webgpu SDL3::SDL3)
