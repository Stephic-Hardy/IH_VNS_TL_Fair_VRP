cmake_minimum_required(VERSION 3.16)
project(TDTSP_Solver VERSION 0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)

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

# quill
find_package(quill QUIET)
if(quill_FOUND)
    message(STATUS "Using system quill")
else()
    include(FetchContent)
    FetchContent_Declare(
            quill
            GIT_REPOSITORY https://github.com/odygrd/quill.git
            GIT_TAG v11.1.0
    )
    FetchContent_MakeAvailable(quill)
    message(STATUS "Using downloaded quill")
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

find_package(Threads REQUIRED)

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

# Library with solver
add_library(tdtsp_lib STATIC ${SOURCES})
set_property(TARGET tdtsp_lib PROPERTY POSITION_INDEPENDENT_CODE TRUE)
target_include_directories(tdtsp_lib PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tdtsp_lib PUBLIC
    quill::quill
)

# Solver executable
add_executable(tdtsp_solver
        src/json_parser.cpp
        src/main.cpp
)
target_include_directories(tdtsp_solver PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tdtsp_solver PRIVATE
    tdtsp_lib
    argparse::argparse
    nlohmann_json::nlohmann_json
    quill::quill
    Threads::Threads
)

# Tests
enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(tdtsp_tests
    tests/cpp/move.cpp
    tests/cpp/neighborhood.cpp
)
target_include_directories(tdtsp_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tdtsp_tests PRIVATE 
    tdtsp_lib
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(tdtsp_tests)

# Pybind stuff
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)
pybind11_add_module(tdtsp_python MODULE bindings/bindings.cpp)
target_include_directories(tdtsp_python PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tdtsp_python PRIVATE tdtsp_lib)

install(TARGETS tdtsp_python DESTINATION tdtsp)
