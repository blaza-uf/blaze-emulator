cmake_minimum_required(VERSION 3.20)

project(blaze-emulator)

option(SDL_VENDORED "Use vendored SDL library" ON)

set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if (MINGW)
	set(CMAKE_C_FLAGS_DEBUG "-g -gdwarf-2 -O0")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-2 -O0")

	# force SDL to use DWARF 2
	set(HAVE_GDWARF_4 OFF CACHE INTERNAL "" FORCE)
endif()

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

add_library(blaze-core OBJECT
	src/core/core.cpp
	src/core/MemRam.cpp
	src/core/CPU.cpp
	src/core/Bus.cpp
	src/core/Register.cpp
	src/core/ROM.cpp
	src/core/MMIO.cpp
	src/core/DMA.cpp
	src/core/SRAM.cpp
	src/core/MulDiv.cpp
)

target_include_directories(blaze-core PUBLIC
	include
	$<TARGET_PROPERTY:SDL2::SDL2-static,INTERFACE_INCLUDE_DIRECTORIES>
)

set(blaze_sources
	src/gui/blaze.cpp
	src/gui/PPU.cpp
	src/gui/APU.cpp
)

if (WIN32)
	set(blaze_sources ${blaze_sources} blaze.rc)
endif()

add_executable(blaze WIN32 ${blaze_sources})

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
	test/support.cpp
)

target_link_libraries(blaze-core-tests PRIVATE blaze-core Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(blaze-core-tests)

set_target_properties(blaze-core blaze blaze-core-tests PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

option(BUILD_SAMPLES "Build SNES ROM samples" OFF)

if (BUILD_SAMPLES)
	add_subdirectory(samples)
endif()
