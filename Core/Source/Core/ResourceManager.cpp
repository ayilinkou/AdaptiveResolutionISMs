#define STB_IMAGE_IMPLEMENTATION
#include "Vendor/stb/stb_image.h"

#include "ResourceManager.h"

namespace Core {
	ResourceManager* ResourceManager::s_Instance = nullptr;

	ResourceManager::~ResourceManager()
	{
		Shutdown();
	}

	void ResourceManager::Shutdown()
	{
		if (!m_ShadersMap.empty())
		{
			__debugbreak(); // Attempting to shutdown when resources are still loaded!
		}

		m_ShadersMap.clear();
		s_Instance = nullptr;
	}

	ID3D11ShaderResourceView* ResourceManager::LoadTexture(const std::string& filepath)
	{
		auto it = m_TexturesMap.find(filepath);
		if (it != m_TexturesMap.end() && it->second.get())
		{
			it->second->AddRef();
			return static_cast<ID3D11ShaderResourceView*>(it->second->m_pData);
		}

		ID3D11ShaderResourceView* pData = Internal_LoadTexture(filepath.c_str());
		if (!pData)
		{
			return nullptr;
		}

		m_TexturesMap[filepath] = std::make_unique<Resource>(pData);
		return pData;
	}

	UINT ResourceManager::UnloadTexture(const std::string& filepath)
	{
		Resource* resourceToUnload = m_TexturesMap[filepath].get();
		if (!resourceToUnload)
		{
			m_TexturesMap.erase(filepath);
			return 0;
		}

		resourceToUnload->RemoveRef();
		if (resourceToUnload->m_RefCount > 0)
		{
			return resourceToUnload->m_RefCount;
		}

		Internal_UnloadTexture(filepath);
		return 0;
	}

	ID3D11ShaderResourceView* ResourceManager::Internal_LoadTexture(const char* filepath)
	{
		int width, height, channels;
		std::string fileString(filepath);
		unsigned char* imageData = stbi_load(filepath, &width, &height, &channels, 0);
		assert(imageData);

		unsigned char* imageDataRgba = imageData;
		bool bNeedsAlpha = channels == 3; // there's no R8G8B8 format so have to use a format with an alpha

		ID3D11Texture2D* texture = nullptr;
		ID3D11ShaderResourceView* textureView = nullptr;

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		if (channels == 1)
		{
			texDesc.Format = DXGI_FORMAT_R8_UNORM;
		}
		else if (channels == 2)
		{
			texDesc.Format = DXGI_FORMAT_R8G8_UNORM;
		}
		else if (channels == 3)
		{
			imageDataRgba = new unsigned char[width * height * 4];

			for (int i = 0; i < width * height; i++)
			{
				imageDataRgba[i * 4 + 0] = imageData[i * 3 + 0];
				imageDataRgba[i * 4 + 1] = imageData[i * 3 + 1];
				imageDataRgba[i * 4 + 2] = imageData[i * 3 + 2];
				imageDataRgba[i * 4 + 3] = 255;
			}

			channels = 4;
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else
		{
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = imageDataRgba;
		initData.SysMemPitch = width * channels;

		assert(SUCCEEDED(m_Device->CreateTexture2D(&texDesc, &initData, &texture)));
		assert(SUCCEEDED(m_Device->CreateShaderResourceView(texture, nullptr, &textureView)));

		if (bNeedsAlpha)
		{
			delete[] imageDataRgba;
		}
		stbi_image_free(imageData);

		NAME_D3D_RESOURCE(texture, (fileString + " texture").c_str());
		NAME_D3D_RESOURCE(textureView, (fileString + " texture SRV").c_str());

		texture->Release();

		return textureView;
	}

	void ResourceManager::Internal_UnloadTexture(const std::string& filepath)
	{
		ID3D11ShaderResourceView* SRV = static_cast<ID3D11ShaderResourceView*>(m_TexturesMap[filepath]->m_pData);
		SRV->Release();
		m_TexturesMap.erase(filepath);
	}
}
