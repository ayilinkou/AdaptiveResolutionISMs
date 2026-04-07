#
# App
#

set(SOURCES
    Source/main.cpp
    Source/AppLayer.h
    Source/AppLayer.cpp
    Source/Shaders/tri.vs
    Source/Shaders/basic.ps
)

add_executable(App)

target_sources(App PRIVATE ${SOURCES})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

target_link_libraries(App Core)

target_include_directories(App PRIVATE Source)

# Set working directory for VS debugger
set_target_properties(App PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/App"
)

# Compiler options
if(MSVC)
    target_compile_options(App PRIVATE
        /W3
        /MP
        /WX
        /ZI
    )
else()
    target_compile_options(App PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
