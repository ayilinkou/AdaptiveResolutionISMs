#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "Texture.h"

struct aiMaterial;

namespace Core {
	class Material
	{
	public:
		Material(aiMaterial* mat, const std::string& texturesRoot);
		~Material() { std::cout << "Material destructor called!" << std::endl; }
		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;
		Material(Material&&) noexcept = default;
		Material& operator=(Material&&) noexcept = default;

		void ProcessMaterial(aiMaterial* mat, const std::string& texturesRoot);

		ID3D11ShaderResourceView* GetAlbedoSRV() const { return m_Albedo->GetSRV(); }

	private:
		std::unique_ptr<Texture> m_Albedo;
		std::unique_ptr<Texture> m_Normal;
		std::unique_ptr<Texture> m_Specular;
		
		std::string m_Name;

		bool m_bTwoSided;
		bool m_bOpaque;
	};
}