cmake_minimum_required(VERSION 3.16)
project(TDTSP_Solver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Компилятор будет создавать исполняемый файл прямо в корне проекта
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/src
)

# Source files
set(SOURCES
    src/data_adapter.cpp
    src/first_step.cpp
    src/insertion_heuristic.cpp
    src/main.cpp
    src/neighborhoods.cpp
    src/tour.cpp
    src/vns_tabu.cpp
    src/json_parser.cpp
    src/problem_arguments.cpp
    src/post_processing.cpp
)

# Create executable
add_executable(tdtsp_solver ${SOURCES})

# Link libraries
target_link_libraries(tdtsp_solver 
    PRIVATE 
        Threads::Threads
)

# Add nlohmann_json
find_package(nlohmann_json 3.2.0 QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(tdtsp_solver PRIVATE nlohmann_json::nlohmann_json)
    message(STATUS "Using system nlohmann_json")
else()
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
    target_link_libraries(tdtsp_solver PRIVATE nlohmann_json::nlohmann_json)
    message(STATUS "Using downloaded nlohmann_json")
endif()