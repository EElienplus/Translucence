# 1. Find dependencies
find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_image REQUIRED)

set(LIB_SOURCES
        src/renderer.cpp
        src/application.cpp
        src/eventSystem.cpp
        src/Input.cpp
)

set(LIB_HEADERS
        include/T_Core.hpp
        include/Renderer.hpp
        include/Application.hpp
        include/eventSystem.hpp
        include/Input.hpp
        include/Math.hpp
        include/imageProcess.hpp
        include/File.hpp
        src/sprite.cpp
        include/sprite.hpp
        src/collider.cpp
        include/collider.hpp
        ../examples/BouncingBall/BouncingBall.cpp
)

add_library(translucence STATIC ${LIB_SOURCES} ${LIB_HEADERS})

target_compile_features(translucence PUBLIC cxx_std_20)

target_include_directories(translucence PUBLIC include)

target_link_libraries(translucence PUBLIC
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        SDL3_image::SDL3_image
)