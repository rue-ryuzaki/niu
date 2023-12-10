if (${CMAKE_VERSION} VERSION_LESS "3.12")
    cmake_minimum_required(VERSION 3.0)
else()
    cmake_minimum_required(VERSION 3.0...3.12)
endif()

include(cmake/prelude.cmake)

project("niu")

include(cmake/warnings.cmake)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include_directories(third_party/stb)

set(ARGPARSE_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/argparse)

set(PNG_EXECUTABLES OFF CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/libpng)

file(GLOB_RECURSE PROJECT_HEADERS "include/*.h*")
file(GLOB_RECURSE PROJECT_SOURCES "src/*.c*")

set(SRC_LIST ${PROJECT_SOURCES} ${PROJECT_HEADERS})

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} argparse::argparse_static)
target_link_libraries(${PROJECT_NAME} png)
