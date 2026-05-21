cmake_minimum_required(VERSION 3.16)
project(TDTSP_Solver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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

# googletest
find_package(GTest 3.2.0 QUIET)
if(GTest_FOUND)
    message(STATUS "Using system GTest")
else()
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.17.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    message(STATUS "Using downloaded GTest")
endif()

# Source files
set(SOURCES
        src/first_step.cpp
        src/insertion_heuristic.cpp
        src/interroute_search.cpp
        src/move.cpp
        src/neighborhood.cpp
        src/route.cpp
        src/route_pack.cpp
        src/vns_tabu.cpp
        src/problem_arguments.cpp
        src/post_processing.cpp
)

# Solver executable
add_executable(tdtsp_solver
        ${SOURCES}
        src/json_parser.cpp
        src/main.cpp
)
target_include_directories(tdtsp_solver PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(tdtsp_solver PRIVATE nlohmann_json::nlohmann_json)

# Library for testing
add_library(tdtsp_lib ${SOURCES})
target_include_directories(tdtsp_lib PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(tests
    test/move.cpp
    test/neighborhood.cpp
)
target_include_directories(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(tests PRIVATE tdtsp_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tests)
