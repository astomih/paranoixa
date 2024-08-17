cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 20)

project(paranoixa)
add_subdirectory(test)
include(webgpu.cmake)
add_subdirectory(library/SDL EXCLUDE_FROM_ALL)
find_package(Vulkan)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Imgui settings
set(CMAKE_CXX_FLAGS "-DIMGUI_IMPL_VULKAN_NO_PROTOTYPES")

if(WIN32)
  set(PARANOIXA_INCLUDE_DIRS
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/library/imgui
    ${CMAKE_SOURCE_DIR}/library/SDL/include
    ${CMAKE_SOURCE_DIR}/include/paranoixa
    ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-wgpu-src/include
    ${CMAKE_SOURCE_DIR}/library/volk
    ${CMAKE_SOURCE_DIR}/library/VulkanMemoryAllocator
  )
endif()

if(EMSCRIPTEN)
  include_directories(
    ${CMAKE_SOURCE_DIR}/library/imgui
    ${CMAKE_SOURCE_DIR}/include/paranoixa
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
    library/imgui/*.cpp
    library/imgui/backends/imgui_impl_sdl3.cpp
    library/imgui/backends/imgui_impl_wgpu.cpp
  )
endif()

if(WIN32)
  file(GLOB
    SOURCE_FILES
    ${SOURCE_FILES}
    library/*.c
    library/*.cpp
    library/imgui/*.cpp
    library/imgui/backends/imgui_impl_sdl3.cpp
    library/imgui/backends/imgui_impl_vulkan.cpp
    library/imgui/backends/imgui_impl_wgpu.cpp
    source/*/*/*.cpp
  )
endif()

find_library(Vulkan SDL3)
add_library(paranoixa STATIC ${SOURCE_FILES})
target_include_directories(paranoixa PUBLIC ${PARANOIXA_INCLUDE_DIRS})
target_link_libraries(paranoixa PUBLIC webgpu SDL3::SDL3)
