# Blaze

An SNES emulator catered toward developing and debugging SNES software.

## Checking Out the Repo

Note that this repository contains submodules. In order to properly clone it,
you must use the `--recursive` flag, like so:

```bash
git clone --recursive https://github.com/blaza-uf/blaze-emulator
```

Alternatively, if you already cloned it without the `--recursive` flag, you can
download the submodules using the following command:

```bash
git submodule update --init --recursive
```

## Building

This project uses CMake as its build system, so you must first configure the
build like so (assuming your current directory is the repository root):

```bash
# configure it first
#
# feel free to replace "Release" with "Debug" here for debug/development builds
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

Configuration only needs to be done once; to build the project use the following command:

```bash
# now build it
cmake --build build
```

Every time you modify a file and want to rebuild the project, simple re-run
this command.

The resulting executable should be called `blaze` or `blaze.exe` (depending on
your OS) somewhere within the `build` directory.
