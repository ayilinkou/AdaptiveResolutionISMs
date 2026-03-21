cmake_minimum_required(VERSION 3.10)

project(AdaptiveResolutionISMs)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Only generate Debug and Release configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

add_subdirectory(Core)
add_subdirectory(App)

# Enable solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY VS_STARTUP_PROJECT App)
