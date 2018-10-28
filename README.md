Introduction
===

Ezio is a library that aims to provide an efficient approach to simplify TCP network programming across platforms.

## Goals

Ezio comes with following goals:
- Support non-blocking I/O via epoll on Linux
- Support asynchronous I/O via I/O Completion Ports on Windows
- Consistent user codebase
- Easy to use

Ezio currently supports TCP socket only.

## Build Instructions

### Windows

**Requirements**

- Windows 7, or later
- Visual Studio 2015, or later (should be C++ 14 compliant)

**Build Steps**

1. Clone the repo with its submodules.
2. Open the `ezio.sln` and build the project `ezio`, the final static library is under the folder `ezio/build/<Configuration>`.
3. Build the `test` project to run test suits as your wish.

Please be noted:
- only x64 version is officially supported
- all projects are originally built with `/MD(d)` flag

### Linux (Ubuntu, specifically)

**Requirements**

- 14.04 LTS x64, or later
- Clang 3.8, or G++ 5.4 as the minimum (should be C++ 14 compliant)
- CMake 3.5, or later
- Python 3.5, or later
- Ninja (optional)

Notes:
- I haven't done compilations for other distributions
- If you do favor using makefile as your build system, you can edit `gen.py` to use makefile.

**Build Steps**

1. Clone the repo with its submodules.
2. Run `gen.py` to build the lib and tests.
   Use `--build-type={Debug|Release}` to specify build mode explicitly; *Debug* is the default mode.
3. If you want to skip building tests, just specify `--build-test=False`

Building the project would not install any of its files into your system's include directory.
