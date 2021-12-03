
cmake_minimum_required(VERSION 3.15)

project(json-cpp-gen LANGUAGES CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE JSON_CPP_GEN_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.h" "generated/*.h")
file(GLOB_RECURSE JSON_CPP_GEN_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp" "generated/*.cpp")

add_executable(json-cpp-gen ${JSON_CPP_GEN_HEADERS} ${JSON_CPP_GEN_SOURCES})
target_compile_features(json-cpp-gen PUBLIC cxx_std_11)
if (MSVC)
    target_compile_definitions(json-cpp-gen PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
