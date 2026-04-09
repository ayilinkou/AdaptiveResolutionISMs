#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "DirectXMath.h"
#include "d3d11.h"
#include "wrl.h"

#include "Texture.h"

struct aiMaterial;

namespace Core {
	struct MaterialData
	{
		DirectX::XMFLOAT3 AlbedoColor = { 1.f, 1.f, 1.f };
		int bHasAlbedoTexture = 0;
		float Specular = 1.f;
		int bHasSpecularTexture = 0;
		float Opacity = 1.f;
		float Padding = 0.f;
	};

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

		Microsoft::WRL::ComPtr<ID3D11Buffer> GetCBuffer() const { return m_ConstantBuffer; }

		ID3D11ShaderResourceView* GetAlbedoSRV() const { if (m_AlbedoTex.get()) return m_AlbedoTex->GetSRV(); return nullptr; }
		ID3D11ShaderResourceView* GetSpecularSRV() const { if (m_SpecularTex) return m_SpecularTex->GetSRV(); return nullptr; }

		bool IsTwoSided() const { return m_bTwoSided; }

	private:
		void CreateCBuffer();

	private:
		std::unique_ptr<Texture> m_AlbedoTex;
		std::unique_ptr<Texture> m_NormalTex;
		std::unique_ptr<Texture> m_SpecularTex;
		
		// this doesn't really need to be stored after cbuffer is made
		MaterialData m_MatData;

		std::string m_Name;

		bool m_bTwoSided;
		float m_Opacity;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	};
}