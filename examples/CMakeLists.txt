cmake_minimum_required(VERSION 4.0)

function(add_example name)
    add_subdirectory(${name})
endfunction()

add_example(BouncingBall)
add_example(UIDemo)
add_example(SmallPlatformer)
