# see /opt/local/share/cmake-3.16/Modules/FindGTest.cmake
find_package(GTest QUIET)

# NOTE: the upper case GTEST! CK
if(NOT GTEST_FOUND)
  # Download and unpack googletest at configure time
  # but only if needed! CK
  configure_file(${CMAKE_SOURCE_DIR}/cmake/GoogleTest-CMakeLists.txt.in
    ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt
  )
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download
  )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download
  )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
    ${CMAKE_BINARY_DIR}/googletest-build
    EXCLUDE_FROM_ALL
  )
endif()

# create libcasm_testing
set(
  libcasm_testing_SOURCES
  ${PROJECT_SOURCE_DIR}/tests/unit/autotools.cc
  ${PROJECT_SOURCE_DIR}/tests/unit/testdir.cc
)
add_library(casm_testing SHARED ${libcasm_testing_SOURCES})
target_include_directories(casm_testing
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)
target_link_libraries(casm_testing
  gtest
  casm_global)
target_compile_options(casm_testing
  PUBLIC
    "-DABS_SRCDIR=\"${CMAKE_SOURCE_DIR}\""
    "-DABS_TOP_BUILDDIR=\"${CMAKE_BINARY_DIR}\""
)

################################################################
# casm_unit_casm_io
add_executable(casm_unit_casm_io
  ${PROJECT_SOURCE_DIR}/tests/unit/gtest_main_run_all.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/casm_io/DataFormatter_test.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/casm_io/EnumIO_test.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/casm_io/Log_test.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/casm_io/jsonParser_test.cpp
)
target_compile_features(casm_unit_casm_io PRIVATE cxx_auto_type)
target_link_libraries(casm_unit_casm_io
  gtest
  casm_global
  casm_testing
)
target_include_directories(casm_unit_casm_io
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)

add_test(NAME casm_unit_casm_io COMMAND casm_unit_casm_io)

################################################################
# casm_unit_container
add_executable(casm_unit_container
  ${PROJECT_SOURCE_DIR}/tests/unit/gtest_main_run_all.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/container/Counter_test.cpp
)
target_compile_features(casm_unit_container PRIVATE cxx_auto_type)
target_link_libraries(casm_unit_container
  gtest
  casm_global
  casm_testing
)
target_include_directories(casm_unit_container
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)
add_test(NAME casm_unit_container COMMAND casm_unit_container)

################################################################
# casm_unit_global
add_executable(casm_unit_global
  ${PROJECT_SOURCE_DIR}/tests/unit/gtest_main_run_all.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/global/version_test.cpp
)
target_compile_features(casm_unit_global PRIVATE cxx_auto_type)
target_link_libraries(casm_unit_global
  gtest
  casm_global
  casm_testing
)
target_include_directories(casm_unit_global
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)
add_test(NAME casm_unit_global COMMAND casm_unit_global)

################################################################
# casm_unit_misc
add_executable(casm_unit_misc
  ${PROJECT_SOURCE_DIR}/tests/unit/gtest_main_run_all.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/misc/string_algorithm_test.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/misc/cloneable_ptr_test.cpp
)
target_compile_features(casm_unit_misc PRIVATE cxx_auto_type)
target_link_libraries(casm_unit_misc
  gtest
  casm_global
  casm_testing
)
target_include_directories(casm_unit_misc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)
add_test(NAME casm_unit_misc COMMAND casm_unit_misc)

################################################################
# casm_unit_system
add_executable(casm_unit_system
  ${PROJECT_SOURCE_DIR}/tests/unit/gtest_main_run_all.cpp
  ${PROJECT_SOURCE_DIR}/tests/unit/system/RuntimeLibrary_test.cpp
)
target_compile_features(casm_unit_system PRIVATE cxx_auto_type)
target_link_libraries(casm_unit_system
  gtest
  casm_global
  casm_testing
)
target_include_directories(casm_unit_system
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unit>
)
add_test(NAME casm_unit_system COMMAND casm_unit_system)
