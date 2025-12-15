add_library(core_resources INTERFACE)

target_sources(core_resources INTERFACE 
               context.hpp
              )

target_include_directories(core_resources INTERFACE 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )