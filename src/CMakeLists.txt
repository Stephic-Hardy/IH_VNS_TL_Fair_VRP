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
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Source files - относительные пути от src/
set(SOURCES
    data_adapter.cpp
    first_step.cpp
    insertion_heuristic.cpp
    main.cpp
    neighborhoods.cpp
    tour.cpp
    vns_tabu.cpp
    json_parser.cpp
    problem_arguments.cpp
    post_processing.cpp
)

# Create executable
add_executable(tdtsp_solver ${SOURCES})

# Link libraries
target_link_libraries(tdtsp_solver 
    PRIVATE 
        Threads::Threads
)

# Add nlohmann_json if available
find_package(nlohmann_json 3.2.0 QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(tdtsp_solver PRIVATE nlohmann_json::nlohmann_json)
    message(STATUS "Using system nlohmann_json")
else()
    # Try to download nlohmann_json
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

# Install target (optional)
install(TARGETS tdtsp_solver RUNTIME DESTINATION bin)