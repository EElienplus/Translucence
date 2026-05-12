# Create the executable
add_executable(app src/main.cpp)

# Link the library. CMake handles all paths automatically.
target_link_libraries(app PRIVATE translucence)