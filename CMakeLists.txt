include(cmake/prelude.cmake)

if (${CMAKE_VERSION} VERSION_LESS "3.12")
    cmake_minimum_required(VERSION 3.0)
else()
    cmake_minimum_required(VERSION 3.0...3.12)
endif()

project("niu")

include(cmake/warnings.cmake)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# argparse
set(ARGPARSE_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/argparse)
# zlib
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/zlib)
set(ZLIB_ROOT third_party/zlib)
set(ZLIB_LIBRARY zlib)
if (NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS ${ZLIB_LIBRARY})
endif()
if (UNIX)
    set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
endif()
# libpng
set(PNG_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
set(PNG_EXECUTABLES OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_EXPORT ON)
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/libpng)

file(GLOB_RECURSE PROJECT_HEADERS "include/*.h*")
file(GLOB_RECURSE PROJECT_SOURCES "src/*.c*")

set(SRC_LIST ${PROJECT_SOURCES} ${PROJECT_HEADERS})

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_include_directories(
    ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(
    ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb)
target_include_directories(
    ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libpng)
target_include_directories(
    ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party/libpng)

target_link_libraries(${PROJECT_NAME} argparse::argparse_static)
target_link_libraries(${PROJECT_NAME} zlibstatic png_static)
