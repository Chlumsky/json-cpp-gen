
cmake_minimum_required(VERSION 3.15)

project(json-cpp-gen-tests LANGUAGES CXX)

add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/json-cpp-gen)

set(GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list(APPEND TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/common.h)

include(basic-case/basic-case.cmake)
include(nested-type-alias/nested-type-alias.cmake)
include(reverse-order-inputs/reverse-order-inputs.cmake)
include(multipass-1/multipass-1.cmake)
include(multipass-2/multipass-2.cmake)
include(multipass-3/multipass-3.cmake)

list(APPEND TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${TEST_FILES} ${GENERATED_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${GEN_ROOT})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_FILES})
source_group(TREE ${GEN_ROOT} FILES ${GENERATED_FILES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
