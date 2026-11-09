cmake_minimum_required(VERSION 3.16)
project(PathfindingVisualizer LANGUAGES CXX)

# Default to Release build if none specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

# C++ Standard requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers (clangd, ccls)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# Dependencies (SFML 2.6)
# -----------------------------------------------------------------------------
find_package(SFML 2.6 COMPONENTS graphics window system QUIET)

if(NOT SFML_FOUND)
    message(STATUS "System SFML 2.6 not found. Fetching SFML 2.6.1 via FetchContent...")
    include(FetchContent)

    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
    )
    FetchContent_MakeAvailable(SFML)
endif()

# -----------------------------------------------------------------------------
# Compiler warnings helper
# -----------------------------------------------------------------------------
function(pfv_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# -----------------------------------------------------------------------------
# Core Library: Grid & Pathfinding Algorithms (SFML-independent)
# -----------------------------------------------------------------------------
add_library(pfv_core STATIC
    src/Grid.cpp
    src/Pathfinder.cpp
    src/Dijkstra.cpp
    src/AStar.cpp
    src/BFS.cpp
    src/DFS.cpp
)

target_include_directories(pfv_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

pfv_apply_warnings(pfv_core)

# -----------------------------------------------------------------------------
# Main Application: SFML GUI & Visualizer
# -----------------------------------------------------------------------------
add_executable(PathfindingVisualizer
    src/main.cpp
    src/App.cpp
    src/Animator.cpp
    src/GridRenderer.cpp
    src/Hud.cpp
    src/InputHandler.cpp
)

target_include_directories(PathfindingVisualizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(TARGET SFML::Graphics)
    target_link_libraries(PathfindingVisualizer PRIVATE
        pfv_core
        SFML::Graphics
        SFML::Window
        SFML::System
    )
else()
    target_link_libraries(PathfindingVisualizer PRIVATE
        pfv_core
        sfml-graphics
        sfml-window
        sfml-system
    )
endif()

pfv_apply_warnings(PathfindingVisualizer)

# Copy asset files to the build output directory
add_custom_command(TARGET PathfindingVisualizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:PathfindingVisualizer>/assets
)

# -----------------------------------------------------------------------------
# Unit Tests
# -----------------------------------------------------------------------------
enable_testing()

add_executable(pfv_tests
    tests/test_algorithms.cpp
)

target_link_libraries(pfv_tests PRIVATE
    pfv_core
)

pfv_apply_warnings(pfv_tests)

# Keep asserts active even in Release configuration
if(MSVC)
    target_compile_options(pfv_tests PRIVATE /UNDEBUG)
else()
    target_compile_options(pfv_tests PRIVATE -UNDEBUG)
endif()

add_test(NAME pfv_algorithms COMMAND pfv_tests)
