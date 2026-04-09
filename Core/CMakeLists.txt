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
	Source/Core/Loaders.h
	Source/Core/Resource.h
	Source/Core/ResourceManager.h
	Source/Core/ResourceManager.cpp
	Source/Core/Logger.h
	Source/Core/Logger.cpp
	Source/Core/Camera.h
	Source/Core/Camera.cpp
	Source/Core/Transform.h
	Source/Core/SceneComponent.h
	Source/Core/Utility.h

	Source/Core/Renderer/Renderer.h
	Source/Core/Renderer/Renderer.cpp
	Source/Core/Renderer/Vertex.h
	Source/Core/Renderer/Mesh.h
	Source/Core/Renderer/Mesh.cpp
	Source/Core/Renderer/Model.h
	Source/Core/Renderer/Model.cpp
	Source/Core/Renderer/ModelData.h
	Source/Core/Renderer/ModelData.cpp
	Source/Core/Renderer/Texture.h
	Source/Core/Renderer/Texture.cpp
	Source/Core/Renderer/TextureData.h
	Source/Core/Renderer/Material.h
	Source/Core/Renderer/Material.cpp
	Source/Core/Renderer/Node.h
	Source/Core/Renderer/Node.cpp
	Source/Core/Renderer/RenderItem.h

	Source/Core/Shader/ShaderResource.h
	Source/Core/Shader/ShaderCreateInfo.h
	Source/Core/Shader/ShaderProgram.h
	Source/Core/Shader/ShaderProgram.cpp
	Source/Core/Shader/ShaderProgramDesc.h
	Source/Core/Shader/ShaderProgramData.h
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
)

target_link_libraries(Core PUBLIC
	assimp::assimp
	d3d11
	dxgi
	d3dcompiler
	dxguid
)