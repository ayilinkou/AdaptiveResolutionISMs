#
# Core
#

set(SOURCES
Source/Core/Application.h
Source/Core/Application.cpp
Source/Core/Window.h
Source/Core/Window.cpp
Source/Core/Layer.h
Source/Core/Event.h
Source/Core/InputEvents.h
Source/Core/InputHandler.h
Source/Core/InputHandler.cpp
)

add_library(Core STATIC)

target_sources(Core PRIVATE ${SOURCES})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

target_include_directories(Core PUBLIC "Source")