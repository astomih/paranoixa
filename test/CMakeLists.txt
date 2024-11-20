cmake_minimum_required(VERSION 3.20)

project(paranoixa_test)
set(CMAKE_CXX_STANDARD 23)
include_directories(
${CMAKE_SOURCE_DIR}/include
${CMAKE_SOURCE_DIR}/library/SDL/include
${CMAKE_SOURCE_DIR}/library/imgui
${CMAKE_SOURCE_DIR}/library/imnodes
)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

if(EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} \
		-Wno-unused-command-line-argument \
		-s WASM=1 \
		-s USE_WEBGPU \
		-s ASYNCIFY=1 \
		--preload-file res")
	add_executable(paranoixa_test test.cpp)
	target_link_libraries(paranoixa_test PRIVATE paranoixa)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if(WIN32)
	link_directories(
		..
	)
	add_executable(paranoixa_test test.cpp)
	target_link_libraries(paranoixa_test PRIVATE paranoixa)
endif()
