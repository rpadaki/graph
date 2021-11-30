cmake_minimum_required(VERSION 3.21)

project(graph)

# Compile and link

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(graph src/main.c)
target_link_libraries(graph ${SDL2_LIBRARIES})

find_program(CLANG_FORMAT NAMES "clang-format")
if (NOT CLANG_FORMAT)
    message(STATUS "clang-format not found")   
endif()

# Format

file(GLOB_RECURSE SOURCE_FILES src/*.c)
file(GLOB_RECURSE HEADER_FILES src/*.h)

add_custom_target(
    format
    COMMAND ${CLANG_FORMAT} -style=file -i ${SOURCE_FILES} ${HEADER_FILES}
    DEPENDS ${SOURCE_FILES} ${HEADER_FILES}
)

# Run
add_custom_target(
    run
    ${CMAKE_BINARY_DIR}/graph
    DEPENDS graph
)
