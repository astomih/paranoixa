cmake_minimum_required(VERSION 3.20)

project(paranoixa_phonon)
set(CMAKE_CXX_STANDARD 23)
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/library/SDL/include
  ${CMAKE_SOURCE_DIR}/library/imgui
  ${CMAKE_SOURCE_DIR}/library/imnodes
)

link_directories(
  ..
)
add_executable(paranoixa_phonon main.cpp)
target_link_libraries(paranoixa_phonon PRIVATE paranoixa)