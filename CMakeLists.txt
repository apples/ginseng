cmake_minimum_required (VERSION 3.0)
project(Ginseng)

file(GLOB_RECURSE ginseng_SOURCES include/ginseng/*)
add_library(ginseng INTERFACE)
target_sources(ginseng INTERFACE ${ginseng_SOURCES})
target_include_directories(ginseng INTERFACE include)

add_executable(test_ginseng EXCLUDE_FROM_ALL src/test.cpp)
set_property(TARGET test_ginseng PROPERTY CXX_STANDARD 14)
target_link_libraries(test_ginseng ginseng)
