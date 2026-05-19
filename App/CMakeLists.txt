add_executable(app src/main.cpp)

target_link_libraries(app PRIVATE translucence)

set_target_properties(app PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)