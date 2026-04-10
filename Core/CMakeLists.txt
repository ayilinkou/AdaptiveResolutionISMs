#
# Core
#

set(SOURCES
	Source/Core/Application/Application.h
	Source/Core/Application/Application.cpp

	Source/Core/Camera/Camera.h
	Source/Core/Camera/Camera.cpp

	Source/Core/Component/Transform.h
	Source/Core/Component/SceneComponent.h

	Source/Core/Event/Event.h
	Source/Core/Event/InputEvents.h

	Source/Core/Input/InputHandler.h
	Source/Core/Input/InputHandler.cpp

	Source/Core/Light/LightManager.h
	Source/Core/Light/LightManager.cpp
	Source/Core/Light/Light.h
	Source/Core/Light/Light.cpp
	Source/Core/Light/PointLight.h
	Source/Core/Light/PointLight.cpp
	Source/Core/Light/SpotLight.h
	Source/Core/Light/SpotLight.cpp
	Source/Core/Light/DirectionalLight.h
	Source/Core/Light/DirectionalLight.cpp

	Source/Core/Layer/Layer.h

	Source/Core/Model/Mesh.h
	Source/Core/Model/Mesh.cpp
	Source/Core/Model/Model.h
	Source/Core/Model/Model.cpp
	Source/Core/Model/ModelData.h
	Source/Core/Model/ModelData.cpp
	Source/Core/Model/ModelSystem.h
	Source/Core/Model/ModelSystem.cpp
	Source/Core/Model/Node.h
	Source/Core/Model/Node.cpp

	Source/Core/Renderer/Renderer.h
	Source/Core/Renderer/Renderer.cpp
	Source/Core/Renderer/RenderQueue.h
	Source/Core/Renderer/RenderQueue.cpp
	Source/Core/Renderer/Vertex.h
	Source/Core/Renderer/Texture.h
	Source/Core/Renderer/Texture.cpp
	Source/Core/Renderer/TextureData.h
	Source/Core/Renderer/Material.h
	Source/Core/Renderer/Material.cpp

	Source/Core/Resource/Loaders.h
	Source/Core/Resource/Resource.h
	Source/Core/Resource/ResourceManager.h
	Source/Core/Resource/ResourceManager.cpp

	Source/Core/Shader/ShaderResource.h
	Source/Core/Shader/ShaderCreateInfo.h
	Source/Core/Shader/ShaderProgram.h
	Source/Core/Shader/ShaderProgram.cpp
	Source/Core/Shader/ShaderProgramDesc.h
	Source/Core/Shader/ShaderProgramData.h

	Source/Core/Utility/MyMacros.h
	Source/Core/Utility/Logger.h
	Source/Core/Utility/Logger.cpp
	Source/Core/Utility/Utility.h
	Source/Core/Utility/Constants.h
	Source/Core/Utility/Timer.h

	Source/Core/Window/Window.h
	Source/Core/Window/Window.cpp
)

set(SHADER_FILES
    Source/Core/Shader/Shaders/GlobalCBuffer.hlsl
    Source/Core/Shader/Shaders/Common.hlsl

    Source/Core/Shader/Shaders/BasicVS.hlsl
    Source/Core/Shader/Shaders/ShadowVS.hlsl

    Source/Core/Shader/Shaders/BasicPS.hlsl
    Source/Core/Shader/Shaders/TransparentPS.hlsl
    Source/Core/Shader/Shaders/PointLightShadowPS.hlsl

    Source/Core/Shader/Shaders/ISMSplatCS.hlsl
)

set_property(SOURCE ${SHADER_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

add_library(Core STATIC)

# Add dependencies
find_package(directxtex CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_sources(Core PRIVATE
    ${SOURCES}
    ${SHADER_FILES}
)

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SHADER_FILES})

target_include_directories(Core PUBLIC "Source")

target_link_libraries(Core PRIVATE
	Microsoft::DirectXTex
)

target_link_libraries(Core PUBLIC
	assimp::assimp
	imgui::imgui
	d3d11
	dxgi
	d3dcompiler
	dxguid
	PointCloudConverter
)