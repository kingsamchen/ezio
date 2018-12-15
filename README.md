Introduction
===

ezio is a library that aims to provide an efficient approach to simplify TCP network programming across platforms.

## Goals

ezio comes with following goals:
- Support non-blocking I/O via epoll on Linux
- Support asynchronous I/O via I/O Completion Ports on Windows
- Consistent user codebase
- Easy to use

ezio currently supports TCP socket only.

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

### Linux

**Requirements**

- Linux kernel 2.6.28 minimum
- Clang 3.8, or G++ 5.4 as the minimum (should be C++ 14 compliant)
- CMake 3.5, or later
- Python 3.5, or later
- Ninja (optional)

Notes:
- Ubuntu is always fully tested, but I haven't done compilations or tests for other distributions
- If you do favor using makefile as your build system, you can specify that when running `gen.py`

**Build Steps**

1. Clone the repo with its submodules.
2. Run `gen.py` to build the lib and tests.
   Use `--build-type={Debug|Release}` to specify build mode explicitly; *Debug* is the default mode.
3. To skip building tests and examples, use `--no-test` and `--no-examples`, respectively

Other detailed options can be found by specifying `--help`.

Building the project would not install any of its files into your system's include directory.

## Acknowledgements

ezio is initially inspired by [muduo](https://github.com/chenshuo/muduo), which is an efficient non-blocking network library, but for Linux only.

Special thanks here to muduo and its author, and also to his remarkable [book](https://book.douban.com/subject/20471211/), which indeed offered great help to me for learning network programming.

Also thank [oceancx](https://github.com/oceancx), one of contributors of ezio, for reporting several critical issues and sharing his thoughtful insight for resolving these issues.