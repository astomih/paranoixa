cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 23)

project(paranoixa)

option(PARANOIXA_BUILD_SDL3 "Build SDL3" ON)

if(PARANOIXA_BUILD_SDL3)
  set(CMAKE_FOLDER "SDL3")
  set(SDL_SHARED OFF)
  set(SDL_STATIC ON)
  add_subdirectory(library/SDL EXCLUDE_FROM_ALL)
  unset(CMAKE_FOLDER)
endif()

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

if(WIN32)
  set(PARANOIXA_USE_WEBGPU ON)

  if(PARANOIXA_USE_WEBGPU)
    set(CMAKE_FOLDER "dawn")
    add_subdirectory(library/WebGPU-distribution EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
  endif()

  set(CMAKE_CXX_FLAGS "-DIMGUI_IMPL_VULKAN_NO_PROTOTYPES -DIMGUI_IMPL_WEBGPU_BACKEND_WGPU")

  find_package(Vulkan REQUIRED)
  set(PARANOIXA_INCLUDE_DIRS
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/library/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/library/imnodes
    ${CMAKE_CURRENT_SOURCE_DIR}/library/SDL/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/paranoixa
    ${CMAKE_CURRENT_SOURCE_DIR}/library/volk
    ${CMAKE_CURRENT_SOURCE_DIR}/library/VulkanMemoryAllocator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/library/tlsf
  )
  file(GLOB_RECURSE
    PARANOIXA_INCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/paranoixa/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/paranoixa/*.hpp
  )
endif()

if(EMSCRIPTEN)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/paranoixa
    ${CMAKE_CURRENT_SOURCE_DIR}/library/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/library/imnodes
    ${CMAKE_CURRENT_SOURCE_DIR}/library/tlsf
  )
endif()

file(GLOB_RECURSE
  SOURCE_FILES
  source/paranoixa/*.hpp
  source/paranoixa/*.cpp
  library/tlsf/tlsf.c
)

if(EMSCRIPTEN)
  file(GLOB
    SOURCE_FILES
    ${SOURCE_FILES}
    library/imgui/*.cpp
    library/imnodes/*.cpp
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
    library/imnodes/*.cpp
    library/imgui/backends/imgui_impl_sdl3.cpp
    library/imgui/backends/imgui_impl_vulkan.cpp
    library/imgui/backends/imgui_impl_dx12.cpp
    library/imgui/backends/imgui_impl_wgpu.cpp
    source/paranoixa/*/*/*.cpp
  )
endif()

add_subdirectory(source/phonon)
add_subdirectory(source/imgui_backend)
add_subdirectory(test)

include_directories(${PARANOIXA_INCLUDE_DIRS})
add_library(paranoixa STATIC ${SOURCE_FILES} ${PARANOIXA_INCLUDE_FILES})

if(EMSCRIPTEN)
  target_link_libraries(paranoixa PRIVATE SDL3::SDL3)
endif()

if(WIN32)
  if(PARANOIXA_USE_WEBGPU)
    target_link_libraries(paranoixa PRIVATE
      webgpu
      SDL3::SDL3
      d3d12
      dxgi
    )
    target_copy_webgpu_binaries(paranoixa)
  else()
    target_link_libraries(paranoixa PRIVATE
      SDL3::SDL3
      d3d12
      dxgi
    )
  endif()
endif()
