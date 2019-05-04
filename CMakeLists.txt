cmake_minimum_required(VERSION 3.11)

# Detect if being bundled via sub-directory.
if(NOT DEFINED PROJECT_NAME)
  set(EZIO_NOT_SUBPROJECT ON)
endif()

project(ezio CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(EZIO_NOT_SUBPROJECT)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set(ROOT_DIR ${CMAKE_SOURCE_DIR})
  set(DEPS_SOURCE_DIR ${ROOT_DIR}/build/deps)
  set(DEPS_DEPLOY_DIR ${CMAKE_BINARY_DIR})
endif()

if(EZIO_NOT_SUBPROJECT)
  set(EZIO_CODE_ANALYSIS_DEFAULT ON)
else()
  set(EZIO_CODE_ANALYSIS_DEFAULT OFF)
endif()

option(EZIO_ENABLE_CODE_ANALYSIS "Enable code analysis" ${EZIO_CODE_ANALYSIS_DEFAULT})
message(STATUS "EZIO_ENABLE_CODE_ANALYSIS = " ${EZIO_ENABLE_CODE_ANALYSIS})

if(EZIO_NOT_SUBPROJECT)
  option(EZIO_BUILD_UNITTESTS "Build ezio unittest" ON)
  message(STATUS "EZIO_BUILD_UNITTEST = " ${EZIO_BUILD_UNITTESTS})

  option(EZIO_BUILD_EXAMPLES "Build ezio examples" ON)
  message(STATUS "EZIO_BUILD_EXAMPLES = " ${EZIO_BUILD_EXAMPLES})
endif()

set(EZIO_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EZIO_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)
set(EZIO_CMAKE_DIR ${EZIO_DIR}/cmake)

include(${EZIO_CMAKE_DIR}/dependency_manager.cmake)

include(${EZIO_CMAKE_DIR}/cotire.cmake)
set(EZIO_PCH_HEADER ${EZIO_DIR}/build/pch/precompile.h)

#Compiler and output configurations.
if(MSVC)
  include(${EZIO_CMAKE_DIR}/compiler_msvc.cmake)
  foreach(OUTPUTCONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG_TYPE} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG_TYPE}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG_TYPE}/lib)
  endforeach()
else()
  include(${EZIO_CMAKE_DIR}/compiler_posix.cmake)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

add_subdirectory(ezio)

if (EZIO_NOT_SUBPROJECT AND EZIO_BUILD_UNITTESTS)
  add_subdirectory(tests)
endif()

if (EZIO_NOT_SUBPROJECT AND EZIO_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
