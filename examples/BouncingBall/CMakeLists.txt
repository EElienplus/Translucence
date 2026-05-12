add_executable(example_BouncingBall BouncingBall.cpp
        ../SmallPlatformer/SmallPlatformer.cpp)
target_link_libraries(example_BouncingBall PRIVATE translucence)