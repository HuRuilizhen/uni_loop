cmake_minimum_required(VERSION 3.15)

project(
  uni_loop
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(APPLE)
  set(UNI_LOOP_SRC src/uni_loop_kqueue.cc)
elseif(UNIX)
  set(UNI_LOOP_SRC src/uni_loop_epoll.cc)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

add_library(uni_loop STATIC ${UNI_LOOP_SRC})
target_include_directories(
  uni_loop PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                  $<INSTALL_INTERFACE:include>)

option(ENABLE_TESTS "Enable tests" OFF)

if(ENABLE_TESTS)
  message(STATUS "Tests enabled")

  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()

  add_executable(uni_loop_basic_test tests/uni_loop_basic_test.cc)
  add_executable(uni_loop_timer_test tests/uni_loop_timer_test.cc)
  add_executable(uni_loop_pipe_test tests/uni_loop_pipe_test.cc)
  target_link_libraries(uni_loop_pipe_test PRIVATE uni_loop GTest::gtest_main)
  target_link_libraries(uni_loop_timer_test PRIVATE uni_loop GTest::gtest_main)
  target_link_libraries(uni_loop_basic_test PRIVATE uni_loop GTest::gtest_main)
  add_test(NAME uni_loop_pipe_test COMMAND uni_loop_pipe_test)
  add_test(NAME uni_loop_timer_test COMMAND uni_loop_timer_test)
  add_test(NAME uni_loop_basic_test COMMAND uni_loop_basic_test)
endif(ENABLE_TESTS)
