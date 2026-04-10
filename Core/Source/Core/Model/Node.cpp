#include "assimp/scene.h"

#include "Node.h"
#include "ModelData.h"

namespace Core
{
	void Node::ProcessNode(aiNode* modelNode, const aiScene* scene, const DirectX::XMMATRIX& parentAccumulatedModelLocal)
	{
		m_NodeName = modelNode->mName.C_Str();
		const DirectX::XMMATRIX localTransform = ConvertToXMMATRIX(modelNode->mTransformation);
		m_AccumulatedModelLocal = parentAccumulatedModelLocal * localTransform;

		m_Meshes.reserve(modelNode->mNumMeshes);
		for (size_t i = 0; i < modelNode->mNumMeshes; i++)
		{
			UINT meshIndex = modelNode->mMeshes[i];
			aiMesh* sceneMesh = scene->mMeshes[meshIndex];
			m_Meshes.push_back(m_pModelData->RegisterMesh(sceneMesh, meshIndex, m_AccumulatedModelLocal)); // TODO: this is already transposed somehow
		}

		for (size_t i = 0; i < scene->mNumLights; i++)
		{
			if (scene->mLights[i]->mName == modelNode->mName)
			{
				m_pModelData->RegisterLight(scene->mLights[i], DirectX::XMMatrixTranspose(m_AccumulatedModelLocal));
			}
		}

		for (size_t i = 0; i < modelNode->mNumChildren; i++)
		{
			m_Children.emplace_back(std::make_unique<Node>(m_pModelData, this));
			m_Children.back().get()->ProcessNode(modelNode->mChildren[i], scene, m_AccumulatedModelLocal);
		}
	}

	DirectX::XMMATRIX Node::ConvertToXMMATRIX(const aiMatrix4x4& matrix) const
	{
		return DirectX::XMMATRIX(&matrix.a1);
	}
}
