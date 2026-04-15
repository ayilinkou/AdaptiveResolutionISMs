#
# App
#

set(SOURCES
    Source/main.cpp

    Source/Layer/AppLayer.h
    Source/Layer/AppLayer.cpp
    Source/Layer/UILayer.h
    Source/Layer/UILayer.cpp

    Source/Scenes.h
)

set(SHADER_FILES
)

set_property(SOURCE ${SHADER_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

add_executable(App)

target_sources(App PRIVATE
    ${SOURCES}
    ${SHADER_FILES}
)

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SHADER_FILES})

target_link_libraries(App Core)

target_include_directories(App PRIVATE Source)

# Set working directory for VS debugger
set_target_properties(App PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/App"
)

compile_options(App)
