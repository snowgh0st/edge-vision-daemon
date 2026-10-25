cmake_minimum_required(VERSION 3.20)
project(EdgeVisionDaemon VERSION 1.0.0 LANGUAGES CXX)

# Enforce C++20 standard across all targets for modern features (concepts, jthreads)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build optimization flags for the Hot Path execution performance
if(MSVC)
    add_compile_options(/O2 /W4 /WX)
else()
    add_compile_options(-O3 -Wall -Wextra -Wpedantic -Werror)
endif()

# Find required system packages: OpenCV for frame capture/processing & Threads
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# Include directories layout
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OpenCV_INCLUDE_DIRS})

# Define the main executable target
add_executable(edge_vision_daemon
    src/main.cpp
)

# Link external libraries (OpenCV modules + Threading library)
target_link_libraries(edge_vision_daemon
    PRIVATE
    ${OpenCV_LIBS}
    Threads::Threads
)