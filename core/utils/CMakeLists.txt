add_library(core_utils INTERFACE)

target_sources(core_utils INTERFACE
    utils.hpp
    # generator expression syntax: $<CONDITION:content_to_add>
    # $<$<BOOL:${CMAKE_CROSSCOMPILING}>:task.hpp>
)

target_include_directories(core_utils INTERFACE 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )

target_link_libraries(core_utils INTERFACE etl)