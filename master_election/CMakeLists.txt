set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required
project(master_election)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -pedantic -fwrapv -pthread")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(SRC_LIST clock.cc clock.h elector-proto.cc elector-proto.h elector.cc elector.h persistent-connection.cc persistent-connection.h synchronization.cc synchronization.h)
add_executable(${PROJECT_NAME} ${SRC_LIST} main.cpp)
add_executable(elector_test ${SRC_LIST} elector_test.cc)