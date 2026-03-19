cmake_minimum_required(VERSION 3.14)
project(Solved2048 LANGUAGES CXX)

# Set the board size as a cache variable, allowing users to configure it when invoking CMake
set(BOARD_SIZE_ROWS 2 CACHE STRING "Number of rows in the 2048 board")
set(BOARD_SIZE_COLS 3 CACHE STRING "Number of columns in the 2048 board")
add_definitions(-DBOARD_SIZE_ROWS=${BOARD_SIZE_ROWS} -DBOARD_SIZE_COLS=${BOARD_SIZE_COLS})
# Export compile commands for clang-tidy and editor tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 1. C++17 Standard Enforcement ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 2. Professional Compiler Flags ---
if(MSVC)
    # MSVC-specific flags
    add_compile_options(/W4 /WX)
else()
    # General flags for all builds
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)

    # Specific flags for Debug mode only
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Build type is Debug: Enabling AddressSanitizer")
        # Add legacy DEBUG macro
        add_compile_definitions(DEBUG)

        # Enable AddressSanitizer for Debug builds
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        add_compile_options(-Og)
        # Note: this is gold standard for easy debug, -O0 for 0 optimization
    endif()
    
    # Flags for Release mode (optimization)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        # Note: can use -02 as well, -03 is more aggressive
    endif()
endif()

# --- 3. Dependency Management (GoogleTest) ---
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
    # GIT_REPOSITORY https://github.com/google/googletest.git
    # GIT_TAG        release-1.12.1
)
# For Windows compatibility
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- 4. Project Layout ---
include_directories(include)

# Define your core library
add_library(core_logic
    src/state.cpp
    src/utils.cpp
)

# Define the main executable
add_executable(solver_2048 src/main.cpp)
# TODO: remove direct calls to tests in main.cpp
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_sources(solver_2048 PRIVATE tests/test_state.cpp)
endif()
target_link_libraries(solver_2048 PRIVATE core_logic)

# --- 5. Unit Testing Setup ---
enable_testing()
add_executable(unit_tests 
    tests/test_state.cpp
)
target_link_libraries(unit_tests PRIVATE core_logic gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)