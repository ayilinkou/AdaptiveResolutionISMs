#include "assimp/material.h"

#include "Material.h"

namespace Core {
	Material::Material(aiMaterial* mat, const std::string& texturesRoot)
	{
		ProcessMaterial(mat, texturesRoot);
	}

	void Material::ProcessMaterial(aiMaterial* mat, const std::string& texturesRoot)
	{
		m_Name = mat->GetName().C_Str();

		mat->Get(AI_MATKEY_TWOSIDED, m_bTwoSided);
		float Opacity = 0.f;
		mat->Get(AI_MATKEY_OPACITY, Opacity);
		m_bOpaque = Opacity >= 1.f;

		aiString path;
		aiColor3D color;
		if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			std::string fullPath = texturesRoot + path.C_Str();
			m_Albedo = std::make_unique<Texture>(fullPath);
		}

		if (mat->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
		{
			std::string fullPath = texturesRoot + path.C_Str();
			m_Normal = std::make_unique<Texture>(fullPath);
		}

		if (mat->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
		{
			std::string fullPath = texturesRoot + path.C_Str();
			m_Specular = std::make_unique<Texture>(fullPath);
		}
	}
}