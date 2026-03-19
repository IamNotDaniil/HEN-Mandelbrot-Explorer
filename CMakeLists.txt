cmake_minimum_required(VERSION 3.16)
project(HENMandelbrotExplorer VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_executable(hen_me
    src/main.cpp
    src/Cli.cpp
    src/ColorMapper.cpp
    src/CpuRenderer.cpp
    src/ImageBuffer.cpp
)

target_include_directories(hen_me PRIVATE include)

if(OpenMP_CXX_FOUND)
    target_link_libraries(hen_me PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(hen_me PRIVATE HENME_HAS_OPENMP=1)
else()
    message(WARNING "OpenMP not found. Building single-threaded CPU renderer.")
endif()

enable_testing()

add_test(
    NAME hen_me_smoke_render
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke_render.py $<TARGET_FILE:hen_me> ${CMAKE_CURRENT_BINARY_DIR}/test_output/render
)

add_test(
    NAME hen_me_smoke_benchmark
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke_benchmark.py $<TARGET_FILE:hen_me> ${CMAKE_CURRENT_BINARY_DIR}/test_output/benchmark
)
