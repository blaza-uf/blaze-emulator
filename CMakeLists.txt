cmake_minimum_required(VERSION 3.20)

project(blaze-emulator)

option(SDL_VENDORED "Use vendored SDL library" ON)

if(SDL_VENDORED)
	add_subdirectory(vendor/sdl EXCLUDE_FROM_ALL)
else()
	# the first line means that the core SDL2 components are required,
	# while the second line means that the SDL2-main component is *not* required.
	#
	# the SDL2-main component is only available on certain platforms (e.g. Windows).
	find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
	find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

add_subdirectory(vendor/catch2 EXCLUDE_FROM_ALL)

add_library(blaze-core STATIC
	src/core/core.cpp
	src/core/MemRam.cpp
	src/core/CPU.cpp
	src/core/Bus.cpp
)

target_include_directories(blaze-core PUBLIC
	include
)

add_executable(blaze WIN32
	src/gui/blaze.cpp
)

target_link_libraries(blaze PRIVATE blaze-core)

# NOTE: according to the SDL CMake documentation (https://wiki.libsdl.org/SDL2/README/cmake),
# SDL2-main (if present) MUST be linked before the core SDL2 components.
if(TARGET SDL2::SDL2main)
	# SDL2-main is required for some platforms like e.g. Windows
	target_link_libraries(blaze PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(blaze PRIVATE SDL2::SDL2-static)

add_executable(blaze-core-tests
	test/color.cpp
	test/cpu.cpp
)

target_link_libraries(blaze-core-tests PRIVATE blaze-core Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(blaze-core-tests)
