#define STB_IMAGE_IMPLEMENTATION

#include "ResourceManager.h"
#include "Core/Renderer/TextureData.h"
#include "Core/Model/ModelData.h"
#include "Loaders.h"

namespace Core {
	ResourceManager* ResourceManager::s_Instance = nullptr;

	ResourceManager::~ResourceManager()
	{
		Shutdown();
	}

	void ResourceManager::Shutdown()
	{
		if (!m_ShadersMap.empty() || !m_TexturesMap.empty())
		{
			__debugbreak(); // Attempting to shutdown when resources are still loaded!
		}
		
		m_ShadersMap.clear();
		m_TexturesMap.clear();

		s_Instance = nullptr;
	}

	TextureData* ResourceManager::LoadTexture(const std::string& filepath)
	{
		auto it = m_TexturesMap.find(filepath);
		if (it != m_TexturesMap.end() && it->second.get())
		{
			it->second->AddRef();
			return static_cast<TextureData*>(it->second->m_pData);
		}

		std::cout << "Loading new texture..." << std::endl;
		TextureData* pData = Loaders::TextureLoader::Load(filepath.c_str(), m_Device.Get(), m_Context.Get());
		if (!pData)
		{
			return nullptr;
		}

		m_TexturesMap[filepath] = std::make_unique<Resource>(pData);
		return pData;
	}

	ModelData* ResourceManager::LoadModel(const std::string& modelPath, const std::string& texturesRoot)
	{
		auto it = m_ModelsMap.find(modelPath);
		if (it != m_ModelsMap.end() && it->second.get())
		{
			it->second->AddRef();
			return static_cast<ModelData*>(it->second->m_pData);
		}

		std::string texturesRootCorrected = texturesRoot;
		if (!texturesRoot.empty() && texturesRoot.back() != '/')
		{
			texturesRootCorrected.append(1, '/');
		}

		std::cout << "Loading new model..." << std::endl;
		ModelData* pData = Loaders::ModelLoader::Load(modelPath.c_str(), texturesRootCorrected.c_str(), m_Device.Get());
		if (!pData)
		{
			return nullptr;
		}

		std::cout << "Model loaded." << std::endl;
		m_ModelsMap[modelPath] = std::make_unique<Resource>(pData);
		return pData;
	}

	ShaderProgramData ResourceManager::LoadShaderProgram(const ShaderProgramDesc& desc)
	{
		ID3D11VertexShader* vs = nullptr;
		ID3D11HullShader* hs = nullptr;
		ID3D11DomainShader* ds = nullptr;
		ID3D11GeometryShader* gs = nullptr;
		ID3D11PixelShader* ps = nullptr;
		ID3D10Blob* vsBlob = nullptr;

		if (!desc.Vertex.Filepath.empty())
			vs = LoadShader<ID3D11VertexShader>(desc.Vertex.Filepath, desc.Vertex.Entry, vsBlob);
		if (!desc.Hull.Filepath.empty())
			hs = LoadShader<ID3D11HullShader>(desc.Hull.Filepath, desc.Hull.Entry);
		if (!desc.Domain.Filepath.empty())
			ds = LoadShader<ID3D11DomainShader>(desc.Domain.Filepath, desc.Domain.Entry);
		if (!desc.Geometry.Filepath.empty())
			gs = LoadShader<ID3D11GeometryShader>(desc.Geometry.Filepath, desc.Geometry.Entry);
		if (!desc.Pixel.Filepath.empty())
			ps = LoadShader<ID3D11PixelShader>(desc.Pixel.Filepath, desc.Pixel.Entry);

		return ShaderProgramData(vs, hs, ds, gs, ps, vsBlob, desc); // as far as I know, this will not make a copy on C++17 and newer
	}

	UINT ResourceManager::UnloadTexture(const std::string& filepath)
	{
		Resource* resourceToUnload = m_TexturesMap[filepath].get();
		if (!resourceToUnload)
		{
			m_TexturesMap.erase(filepath);
			return 0u;
		}

		resourceToUnload->RemoveRef();
		if (resourceToUnload->m_RefCount > 0u)
		{
			return resourceToUnload->m_RefCount;
		}

		Internal_UnloadTexture(filepath);
		return 0u;
	}

	UINT ResourceManager::UnloadModel(const std::string& filepath)
	{
		Resource* ResourceToUnload = m_ModelsMap[filepath].get();
		if (!ResourceToUnload)
		{
			m_ModelsMap.erase(filepath);
			return 0u;
		}

		ResourceToUnload->RemoveRef();
		if (ResourceToUnload->m_RefCount > 0u)
		{
			return ResourceToUnload->m_RefCount;
		}

		Internal_UnloadModel(filepath);
		return 0u;
	}

	void ResourceManager::UnloadShaderProgram(const ShaderProgramDesc& desc)
	{
		if (!desc.Vertex.Filepath.empty())
			UnloadShader<ID3D11VertexShader>(desc.Vertex.Filepath, desc.Vertex.Entry);
		if (!desc.Hull.Filepath.empty())
			UnloadShader<ID3D11HullShader>(desc.Hull.Filepath, desc.Hull.Entry);
		if (!desc.Domain.Filepath.empty())
			UnloadShader<ID3D11DomainShader>(desc.Domain.Filepath, desc.Domain.Entry);
		if (!desc.Geometry.Filepath.empty())
			UnloadShader<ID3D11GeometryShader>(desc.Geometry.Filepath, desc.Geometry.Entry);
		if (!desc.Pixel.Filepath.empty())
			UnloadShader<ID3D11PixelShader>(desc.Pixel.Filepath, desc.Pixel.Entry);
	}

	void ResourceManager::Internal_UnloadTexture(const std::string filepath)
	{
		TextureData* pTextureData = static_cast<TextureData*>(m_TexturesMap[filepath]->m_pData);
		delete pTextureData;
		m_TexturesMap.erase(filepath);
	}

	void ResourceManager::Internal_UnloadModel(const std::string filepath)
	{
		ModelData* pModelData = static_cast<ModelData*>(m_ModelsMap[filepath]->m_pData);
		delete pModelData;
		m_ModelsMap.erase(filepath);
	}
}
