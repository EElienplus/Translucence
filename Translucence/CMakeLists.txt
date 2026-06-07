# 1. Find dependencies
find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)

set(LIB_SOURCES
        src/Renderer.cpp
        src/Application.cpp
        src/EventSystem.cpp
        src/Input.cpp
        src/ImageProcess.cpp
        src/Sprite.cpp
        src/Collider.cpp
        src/Audio.cpp
        src/MwFile.cpp
        src/Mtf.cpp
        src/ParticleEmitter.cpp
        src/Graph.cpp
)

set(LIB_HEADERS
        include/T_Core.hpp
        include/Renderer.hpp
        include/Application.hpp
        include/EventSystem.hpp
        include/Input.hpp
        include/Math.hpp
        include/ImageProcess.hpp
        include/File.hpp
        include/Sprite.hpp
        include/Collider.hpp
        include/Audio.hpp
        include/MwFile.hpp
        include/Mtf.hpp
        include/ParticleEmitter.hpp
        src/UiPanel.cpp
        include/UiPanel.hpp
        include/Graph.hpp
)

add_library(translucence STATIC ${LIB_SOURCES} ${LIB_HEADERS})

target_compile_features(translucence PUBLIC cxx_std_20)

target_include_directories(translucence PUBLIC include)

target_link_libraries(translucence PUBLIC
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
)

if(APPLE)
    target_link_libraries(translucence PUBLIC "-framework Foundation" "-framework QuartzCore")
endif()

target_compile_definitions(translucence PRIVATE TRANSLUCENCE_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/")