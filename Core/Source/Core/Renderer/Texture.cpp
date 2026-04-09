#include <format>

#include "Texture.h"
#include "TextureData.h"
#include "Core/Resource/ResourceManager.h"
#include "Core/Utility/Logger.h"

namespace Core {
	Texture::Texture(const std::string& filepath)
	{
		Init(filepath);
	}

	Texture::Texture(const Texture& other)
	{
		std::cout << "Texture's copy constructor..." << std::endl;
		Init(other.m_pTextureData->m_Filepath);
	}

	Texture& Texture::operator=(const Texture& other)
	{
		std::cout << "Texture's copy assignment..." << std::endl;
		if (other.m_pTextureData)
		{
			Init(other.m_pTextureData->m_Filepath);
		}
		return *this;
	}

	Texture::Texture(Texture&& other) noexcept
		: m_pTextureData(other.m_pTextureData)
	{
		std::cout << "Texture's move constructor..." << std::endl;
		other.m_pTextureData = nullptr;
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		std::cout << "Texture's move assignment..." << std::endl;
		if (this != &other)
		{
			m_pTextureData = other.m_pTextureData;
			other.m_pTextureData = nullptr;
		}
		return *this;
	}

	Texture::~Texture()
	{
		std::cout << "Texture being destroyed..." << std::endl;
		if (m_pTextureData)
			Core::ResourceManager::Get()->UnloadTexture(m_pTextureData->m_Filepath);
	}

	ID3D11ShaderResourceView* Texture::GetSRV()
	{
		if (!m_pTextureData)
			return nullptr;
		return m_pTextureData->m_SRV.Get();
	}

	void Texture::Init(const std::string& filepath)
	{
		m_pTextureData = Core::ResourceManager::Get()->LoadTexture(filepath);

		if (!m_pTextureData)
		{
			std::string message = std::format("Failed to load texture {}", filepath);
			Logger::ShowMessageBox(message.c_str(), "Failed to load texture!", MB_OK | MB_ICONERROR);
			std::abort(); // maybe this is overkill but it keeps the program safe
		}
	}
}
