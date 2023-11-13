# This toolchain file allows Blaze to be compiled for Windows on Linux
#
# You can use it by adding the following to your CMake configuration command:
#     -DCMAKE_TOOLCHAIN_FILE=<path-to-blaze-repo>/cmake/linux-mingw.cmake

set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

set(CMAKE_FIND_ROOT_PATH
	/usr/x86_64-w64-mingw32
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -static")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -static")
