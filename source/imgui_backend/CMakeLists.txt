cmake_minimum_required(VERSION 3.20)

project(paranoixa_imgui_backend)
set(CMAKE_CXX_STANDARD 23)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../library/imgui
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
add_library(paranoixa_imgui_backend ${SOURCES})
target_link_libraries(paranoixa_imgui_backend PRIVATE paranoixa)