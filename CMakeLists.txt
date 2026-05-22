cmake_minimum_required(VERSION 4.0)
project(TranslucenceWorkspace LANGUAGES CXX)

# Ensure Homebrew paths are searched on macOS
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
endif()

add_subdirectory(Translucence)
add_subdirectory(App)
add_subdirectory(examples)