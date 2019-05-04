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

### Platform Requirements

#### Windows

- Windows 7, or later
- Visual Studio 2015, or later (C++ 14 is required)
- CMake 3.11 or later
- Python 3

Note:

- If python 3 was not installed, you should run cmake configuration and build targets manually.
- We will use the latest version of Visual Studio as possible and use x64 as the default target architecture.

#### Ubuntu

- 14.04 LTS x64, or later
- Clang 3.8, or G++ as the minimum (C++ 14 is required)
- CMake 3.11, or later
- Python 3
- Ninja (optional)

Note:

- If python 3 was not installed, you should run cmake configuration and build targets manually.
- If Ninja was not installed, you can use the traditional Makefile

### Generate & Build

KBase uses [anvil](https://github.com/kingsamchen/anvil) to assist in generating build system files and running builds.

Please be noted that, building the project on Linux platforms would not install any of its files into your system's include directory.

Run `anvil --help` to check command flags in details.

## Acknowledgements

ezio is initially inspired by [muduo](https://github.com/chenshuo/muduo), which is an efficient non-blocking network library, but for Linux only.

Special thanks here to muduo and its author, and also to his remarkable [book](https://book.douban.com/subject/20471211/), which indeed offered great help to me for learning network programming.

Also thank [oceancx](https://github.com/oceancx), one of contributors of ezio, for reporting several critical issues and sharing his thoughtful insight for resolving these issues.