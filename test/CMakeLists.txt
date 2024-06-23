cmake_minimum_required(VERSION 3.10)

project(test)
include_directories(${CMAKE_SOURCE_DIR}/include)

if(EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} \
		-Wno-unused-command-line-argument \
		-s WASM=1 \
		-s USE_WEBGPU=1 \
		-s USE_SDL=2 \
		-s ASYNCIFY=1 \
		-std=c++20")
	add_executable(test test.cpp)
	target_link_libraries(test PRIVATE paranoixa)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if(WIN32)
	set(CMAKE_CXX_FLAGS "-g -O0")
	find_package(Vulkan REQUIRED)
	find_package(SDL2 REQUIRED)
	add_executable(test test.cpp)
	target_link_libraries(test PRIVATE paranoixa SDL2 SDL2main webgpu)
endif()
