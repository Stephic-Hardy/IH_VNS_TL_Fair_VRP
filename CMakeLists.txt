cmake_minimum_required(VERSION 3.16)
project(TDTSP_Solver VERSION 0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Executable will be created in the root of the repo
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -O2)
endif()


# Find required packages

# argparse
find_package(argparse 3.2 QUIET)
if(argparse_FOUND)
    message(STATUS "Using system argparse")
else()
    include(FetchContent)
    FetchContent_Declare(
            argparse
            GIT_REPOSITORY https://github.com/p-ranav/argparse.git
            GIT_TAG v3.2
    )
    FetchContent_MakeAvailable(argparse)
    message(STATUS "Using downloaded argparse")
endif()

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
        src/optimizer.cpp
        src/initializer.cpp
        src/route.cpp
        src/route_pack.cpp
        src/vns_tabu.cpp
        src/problem_arguments.cpp
        src/post_processing.cpp
        src/solver.cpp
        src/utils.cpp
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
target_link_libraries(tdtsp_solver PRIVATE
    argparse::argparse
    nlohmann_json::nlohmann_json
)

# Library for testing
add_library(tdtsp_lib SHARED ${SOURCES})
set_property(TARGET tdtsp_lib PROPERTY POSITION_INDEPENDENT_CODE TRUE) 
target_include_directories(tdtsp_lib PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(tdtsp_tests
    tests/cpp/move.cpp
    tests/cpp/neighborhood.cpp
)
target_include_directories(tdtsp_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(tdtsp_tests PRIVATE tdtsp_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tdtsp_tests)

# Pybind stuff
# Partially yoinked from https://bastian.rieck.me/blog/2018/cmake_cpp_pybind11_hard_mode/
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

pybind11_add_module(_tdtsp MODULE bindings/bindings.cpp ${SOURCES})
target_include_directories(_tdtsp PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

install(TARGETS _tdtsp DESTINATION tdtsp)
