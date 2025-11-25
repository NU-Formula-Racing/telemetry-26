add_subdirectory(can)
add_subdirectory(lora)
add_subdirectory(sd)

add_library(core_drivers INTERFACE)

target_link_libraries(core_drivers INTERFACE
                      core_can
                      core_lora
                      core_sd
                    )