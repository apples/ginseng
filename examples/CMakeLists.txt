cmake_minimum_required (VERSION 3.0)
project(GinsengExamples)

function(ginseng_add_example example)
    add_executable(ginseng_${example} ${CMAKE_CURRENT_SOURCE_DIR}/${example}.cpp)
    set_property(TARGET ginseng_${example} PROPERTY CXX_STANDARD 17)
    target_link_libraries(ginseng_${example} ginseng)
endfunction()

ginseng_add_example(enemy_game)
ginseng_add_example(everything)
ginseng_add_example(position_velocity)
