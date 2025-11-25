cmake_minimum_required(VERSION 3.28)

project(telemetry 
        VERSION 0.0 
        LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_CROSSCOMPILING)
    message(STATUS "Build mode: TARGET (STM32)")
    if(NOT CMAKE_TOOLCHAIN_FILE)
        message(FATAL_ERROR "Target builds require a toolchain file. \
                             Set -DCMAKE_TOOLCHAIN_FILE=path/to/arm-none-eabi.cmake")
    endif()
else()
    message(STATUS "Build mode: NATIVE (Host)")
    enable_testing()
    add_subdirectory(third-party/googletest)
    include(GoogleTest)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    include(CTest)
    include(tools/cmake/format.cmake)
endif()

add_subdirectory(core)
add_subdirectory(base-station)
add_subdirectory(remote)