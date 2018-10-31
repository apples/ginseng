cmake_minimum_required (VERSION 3.0)
project(Ginseng)

set(GINSENG_BUILD_EXAMPLES No CACHE BOOL "Build examples")

add_library(ginseng INTERFACE)
set_property(TARGET ginseng PROPERTY INTERFACE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/include/ginseng/ginseng.hpp)
target_include_directories(ginseng INTERFACE include)

add_executable(test_ginseng EXCLUDE_FROM_ALL
  src/main.cpp
  src/test.cpp
  src/catch.hpp
  src/test_tags.cpp
  src/test_primary.cpp
  src/test_stress.cpp
  src/test_bitset.cpp)
set_property(TARGET test_ginseng PROPERTY CXX_STANDARD 17)
target_link_libraries(test_ginseng ginseng)

if(GINSENG_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
