cmake_minimum_required(VERSION 3.22.1)

project(stockAlgos)

set(CPP_VERSION 17)
set(CMAKE_SUPPRESS_REGENERATION true)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/utilities/cmake/")
include(gameSources)
include(macros)

GENERATE_SOURCE_GROUPS("${CMAKE_SOURCE_DIR}/src/" ${SOURCES})

include_directories(src)

set(CMAKE_CXX_FLAGS  "-W4 -EHsc" )
set(CMAKE_CXX_FLAGS_RELEASE "-Ox -GL")
add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD ${CPP_VERSION})