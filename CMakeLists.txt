cmake_minimum_required(VERSION 3.16)
project(tdtsp_solver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable will be created in the root of the repo
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -O2)
endif()


# Find required packages

# nlohmann_json
find_package(nlohmann_json 3.2.0 QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Using system nlohmann_json")
else()
    include(FetchContent)
    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
    message(STATUS "Using downloaded nlohmann_json")
endif()

# Source files
set(SOURCES
        src/first_step.cpp
        src/insertion_heuristic.cpp
        src/main.cpp
        src/move.cpp
        src/neighborhood.cpp
        src/tour.cpp
        src/route_pack.cpp
        src/vns_tabu.cpp
        src/json_parser.cpp
        src/problem_arguments.cpp
        src/post_processing.cpp
)

add_executable(tdtsp_solver ${SOURCES})

# Include directories
target_include_directories(tdtsp_solver PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(tdtsp_solver 
    PRIVATE
        nlohmann_json::nlohmann_json
)
