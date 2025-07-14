cmake_minimum_required(VERSION 3.20)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

project(CASMcode_global_tests VERSION 2.2.0 LANGUAGES CXX)


# set CMAKE_INSTALL_X variables
include(GNUInstallDirs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# try to use ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

### googletest ###
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(googletest)
message(STATUS "googletest_SOURCE_DIR: ${googletest_SOURCE_DIR}")

# compile Google Test as an object library
add_library(gtest_all OBJECT "${googletest_SOURCE_DIR}/googletest/src/gtest-all.cc")
set_property(TARGET gtest_all PROPERTY INCLUDE_DIRECTORIES
  "${googletest_SOURCE_DIR}/googletest/include"
  "${googletest_SOURCE_DIR}/googletest")
set_property(TARGET gtest_all PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_options(gtest_all
  PUBLIC
    "-DGTEST_HAS_PTHREAD=0"
    "-DGTEST_LINKED_AS_SHARED_LIBRARY=1"
)

### ZLIB ###
# Should find ZLIB::ZLIB
find_package(ZLIB)


### CASM ###

# Find CASM
if(NOT DEFINED CASM_PREFIX)
  message(FATAL_ERROR "CASM_PREFIX not defined")
endif()
if(DEFINED CASM_PREFIX)
  set(CASMcode_global_ROOT ${CASM_PREFIX}/share/CASMcode_global/cmake)
endif()

find_package(CASMcode_global)
if(NOT CASMcode_global_FOUND)
  message(FATAL_ERROR "CMake failed to find CASMcode_global")
endif()
# if successful, we have CASM::casm_global


### libcasm_testing ###
set(
  libcasm_testing_SOURCES
  ${PROJECT_SOURCE_DIR}/unit/autotools.cc
  ${PROJECT_SOURCE_DIR}/unit/testdir.cc
)
add_library(casm_testing SHARED ${libcasm_testing_SOURCES})
target_include_directories(casm_testing
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
    $<BUILD_INTERFACE:${googletest_SOURCE_DIR}/googletest/include>
)
target_link_libraries(casm_testing
  gtest_all
  CASM::casm_global
)
target_compile_options(casm_testing
  PUBLIC
    "-DABS_SRCDIR=\"${CMAKE_SOURCE_DIR}\""
    "-DABS_TOP_BUILDDIR=\"${CMAKE_BINARY_DIR}\""
)

enable_testing()

################################################################
# casm_unit_casm_io
add_executable(casm_unit_casm_io
  ${PROJECT_SOURCE_DIR}/unit/casm_io/DataFormatter_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/casm_io/EnumIO_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/casm_io/Log_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/casm_io/jsonParser_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/gtest_main_run_all.cpp
)
target_link_libraries(casm_unit_casm_io
  gtest_all
  CASM::casm_global
  casm_testing
  ZLIB::ZLIB
)
target_include_directories(casm_unit_casm_io
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)

add_test(NAME casm_unit_casm_io COMMAND casm_unit_casm_io)

################################################################
# casm_unit_container
add_executable(casm_unit_container
  ${PROJECT_SOURCE_DIR}/unit/container/Counter_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/gtest_main_run_all.cpp
)
target_link_libraries(casm_unit_container
  gtest_all
  CASM::casm_global
  casm_testing
  ZLIB::ZLIB
)
target_include_directories(casm_unit_container
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)

add_test(NAME casm_unit_container COMMAND casm_unit_container)

################################################################
# casm_unit_global
add_executable(casm_unit_global
  ${PROJECT_SOURCE_DIR}/unit/global/version_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/gtest_main_run_all.cpp
)
target_link_libraries(casm_unit_global
  gtest_all
  CASM::casm_global
  casm_testing
  ZLIB::ZLIB
)
target_include_directories(casm_unit_global
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)

add_test(NAME casm_unit_global COMMAND casm_unit_global)

################################################################
# casm_unit_misc
add_executable(casm_unit_misc
  ${PROJECT_SOURCE_DIR}/unit/gtest_main_run_all.cpp
  ${PROJECT_SOURCE_DIR}/unit/misc/CASM_math_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/misc/cloneable_ptr_test.cpp
  ${PROJECT_SOURCE_DIR}/unit/misc/string_algorithm_test.cpp
)
target_link_libraries(casm_unit_misc
  gtest_all
  CASM::casm_global
  casm_testing
  ZLIB::ZLIB
)
target_include_directories(casm_unit_misc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)

add_test(NAME casm_unit_misc COMMAND casm_unit_misc)

################################################################
# casm_unit_system
add_executable(casm_unit_system
  ${PROJECT_SOURCE_DIR}/unit/gtest_main_run_all.cpp
  ${PROJECT_SOURCE_DIR}/unit/system/RuntimeLibrary_test.cpp
)
target_link_libraries(casm_unit_system
  gtest_all
  CASM::casm_global
  casm_testing
  ZLIB::ZLIB
)
target_include_directories(casm_unit_system
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)

add_test(NAME casm_unit_system COMMAND casm_unit_system)
