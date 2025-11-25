cmake_minimum_required(VERSION 3.28)

project(telemetry-remote 
        VERSION 0.0 
        LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# only run linter on native builds
if(NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

add_subdirectory(../core ${CMAKE_BINARY_DIR}/core)
add_subdirectory(src/app)

if(CMAKE_CROSSCOMPILING)
    message(STATUS "Building remote for TARGET (STM32)...")
    if(NOT CMAKE_TOOLCHAIN_FILE)
        message(FATAL_ERROR "Target builds require a toolchain file. \
                             Set -DCMAKE_TOOLCHAIN_FILE=path/to/arm-none-eabi.cmake")
    endif()
    
    # compiler flags for STM32 - ARM Cortex-M4
    add_compile_options(-mcpu=cortex-m4
                        -mthumb
                        -mfpu=fpv4-sp-d16
                        -mfloat-abi=hard
                        -fdata-sections
                        -ffunction-sections
                        -Wall
                        -Wextra
                        $<$<CONFIG:Debug>:-Og>
                       )

    # linker flags for STM32 - ARM Cortex-M4
    add_link_options(-mcpu=cortex-m4
                     -mthumb
                     -mfpu=fpv4-sp-d16
                     -mfloat-abi=hard
                     --specs=nosys.specs 
                     # --specs=nano.specs # smaller binary size
                     -Wl,--gc-sections
                     -Wl,--print-memory-usage
                    # -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F405RGTx_FLASH.ld
                    )

    # .elf target for remote
    add_executable(remote.elf
                   src/main.cpp)               

    target_include_directories(remote.elf PRIVATE 
                               ${CMAKE_SOURCE_DIR}/src/app
                              )
 
    target_link_libraries(remote.elf PRIVATE
                            remote_app
                            core_bsp
                            core_drivers
                            core_interfaces
                         )
 
    # convert .elf to flashable .bin and .hex files with objcopy
    find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy)
 
    add_custom_command(
        TARGET remote.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:remote.elf> $<TARGET_FILE_DIR:remote.elf>/remote.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:remote.elf> $<TARGET_FILE_DIR:remote.elf>/remote.hex
        COMMENT "Generating remote .bin and .hex files"
    )

else()
    message(STATUS "Building remote for NATIVE (Host)...")
    
    # add unit testing + linting for native builds
    enable_testing()
    
    # disable linting for third-party code
    set(TEMP_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
    set(CMAKE_CXX_CLANG_TIDY "")
    add_subdirectory(../third-party/googletest ${CMAKE_BINARY_DIR}/googletest)
    set(CMAKE_CXX_CLANG_TIDY ${TEMP_CLANG_TIDY})

    include(GoogleTest)
    include(CTest)
    include(../tools/cmake/format.cmake)

    add_subdirectory(tests)
endif()
