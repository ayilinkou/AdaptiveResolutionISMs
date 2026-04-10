#
# PointCloudConverter
#

set(SOURCES
	Source/PointCloudConverter/Converter.h
	Source/PointCloudConverter/Converter.cpp
)

add_library(PointCloudConverter STATIC)

target_sources(PointCloudConverter PRIVATE ${SOURCES})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

target_include_directories(PointCloudConverter PUBLIC "Source")

target_link_libraries(PointCloudConverter PUBLIC Core)