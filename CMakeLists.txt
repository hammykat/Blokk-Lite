cmake_minimum_required(VERSION 3.15)
project(BlokkLite)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build Raylib from source in vendor/raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/raylib)
add_subdirectory(vendor/xsimd)

# Gather all source files in src/
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_library(BlokkLite STATIC ${SOURCES})

# Include header paths
target_include_directories(BlokkLite PUBLIC
    include
    src
    vendor/miniaudio
    vendor/tsl
)

# Link libraries
target_link_libraries(BlokkLite PUBLIC 
    raylib
    xsimd
)

