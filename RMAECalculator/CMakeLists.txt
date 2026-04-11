#
# RMAECalculator
#

set(SOURCES
    Source/RMAECalculator/main.cpp
    Source/RMAECalculator/RMAECalculator.h
    Source/RMAECalculator/RMAECalculator.cpp
)

add_executable(RMAECalculator)

target_sources(RMAECalculator PRIVATE ${SOURCES})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

target_include_directories(RMAECalculator PRIVATE Source)

# Set working directory for VS debugger
set_target_properties(RMAECalculator PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/RMAECalculator"
)

# Compiler options
if(MSVC)
    target_compile_options(RMAECalculator PRIVATE
        /W3
        /MP
        /WX
        /ZI
    )
else()
    target_compile_options(RMAECalculator PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
