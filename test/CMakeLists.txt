cmake_minimum_required(VERSION 3.10)

project(test)
include_directories(${CMAKE_SOURCE_DIR}/include)

if(EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} \
		-Wno-unused-command-line-argument \
		-s WASM=1 \
		-s USE_WEBGPU=1 \
		-s ASYNCIFY=1 \
		-std=c++20")
	add_executable(test test.cpp)
	target_link_libraries(test PRIVATE paranoixa)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if(WIN32)
	link_directories(
		..
	)
	add_executable(test test.cpp)
	target_link_libraries(test PRIVATE paranoixa)
endif()
