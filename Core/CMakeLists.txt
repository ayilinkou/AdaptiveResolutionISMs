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
	Source/Core/MyMacros.h
	Source/Core/ResourceManager.h
	Source/Core/ResourceManager.cpp
	Source/Core/Resource.h
	Source/Core/Logger.h
	Source/Core/Logger.cpp
	Source/Core/Camera.h
	Source/Core/Camera.cpp
	Source/Core/Transform.h
	Source/Core/SceneComponent.h
	Source/Core/Utility.h

	Source/Core/Renderer/Renderer.h
	Source/Core/Renderer/Renderer.cpp

	Source/Core/Shader/ShaderResource.h
	Source/Core/Shader/ShaderCreateInfo.h
)

add_library(Core STATIC)

# Add dependencies
find_package(directxtex CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

target_sources(Core PRIVATE ${SOURCES})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

target_include_directories(Core PUBLIC "Source")

target_link_libraries(Core PRIVATE
	Microsoft::DirectXTex
	assimp::assimp
)

target_link_libraries(Core PUBLIC
	d3d11
	dxgi
	d3dcompiler
	dxguid
)