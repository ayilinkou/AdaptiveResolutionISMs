#include "assimp/mesh.h"

#include "Mesh.h"
#include "ModelData.h"
#include "Vertex.h"

namespace Core {
	void Mesh::Init(ModelData* pModelData, aiMesh* mesh)
	{
		m_pModelData = pModelData;
		m_Name = mesh->mName.C_Str();
		UINT verticesOffset = (UINT)m_pModelData->m_Vertices->size();
		m_IndexOffset = (UINT)m_pModelData->m_Indices->size();
		m_Material = &pModelData->m_Materials[mesh->mMaterialIndex];

		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex v;
			v.Pos = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			v.Normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

			if (mesh->mTextureCoords[0])
			{
				v.TexCoord = DirectX::XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}
			else
			{
				v.TexCoord = DirectX::XMFLOAT2(0.f, 0.f);
			}

			m_pModelData->m_Vertices->push_back(v);
		}

		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
			{
				m_pModelData->m_Indices->push_back(face.mIndices[j] + verticesOffset);
			}
		}
		m_IndexCount = (UINT)m_pModelData->m_Indices->size() - m_IndexOffset;
	}
}