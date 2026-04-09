#include "assimp/material.h"

#include "Material.h"
#include "Renderer.h"
#include "Core/Utility/MyMacros.h"

namespace Core {
	Material::Material(aiMaterial* mat, const std::string& texturesRoot)
	{
		ProcessMaterial(mat, texturesRoot);
	}

	void Material::ProcessMaterial(aiMaterial* mat, const std::string& texturesRoot)
	{
		m_Name = mat->GetName().C_Str();

		mat->Get(AI_MATKEY_TWOSIDED, m_bTwoSided);
		mat->Get(AI_MATKEY_OPACITY, m_Opacity);
		m_MatData.Opacity = m_Opacity;

		aiString path;
		aiColor3D color;
		if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			std::string fullPath = texturesRoot + path.C_Str();
			m_AlbedoTex = std::make_unique<Texture>(fullPath);
			m_MatData.bHasAlbedoTexture = true;
		}
		else if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		{
			m_MatData.AlbedoColor.x = color.r;
			m_MatData.AlbedoColor.y = color.g;
			m_MatData.AlbedoColor.z = color.b;
		}

		// TODO: do I need this? I get normals from vertices
		if (mat->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
		{
			std::string fullPath = texturesRoot + path.C_Str();
			m_NormalTex = std::make_unique<Texture>(fullPath);
		}

		if (mat->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
		{
			std::string fullPath = texturesRoot + path.C_Str();
			m_SpecularTex = std::make_unique<Texture>(fullPath);
			m_MatData.bHasSpecularTexture = true;
		}
		else if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
		{
			m_MatData.Specular = color.r;
		}

		CreateCBuffer();
	}

	void Material::CreateCBuffer()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Renderer::Get()->GetDevice().Get();
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(MaterialData);

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = &m_MatData;

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, &data, &m_ConstantBuffer));
		NAME_D3D_RESOURCE(m_ConstantBuffer, (m_Name + " material constant buffer").c_str());
	}
}