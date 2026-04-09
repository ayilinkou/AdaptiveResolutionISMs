#include "assimp/scene.h"

#include "Node.h"
#include "ModelData.h"

namespace Core
{
	void Node::ProcessNode(aiNode* modelNode, const aiScene* scene)
	{
		m_NodeName = modelNode->mName.C_Str();

		DirectX::XMMATRIX localTransform = ConvertToXMMATRIX(modelNode->mTransformation);
		const DirectX::XMMATRIX& parentAccumulatedTransform = m_ParentNode ? m_ParentNode->GetAccumulatedTransform() : DirectX::XMMatrixIdentity();
		m_AccumulatedTransform = parentAccumulatedTransform * localTransform;

		for (size_t i = 0; i < modelNode->mNumMeshes; i++)
		{
			UINT meshIndex = modelNode->mMeshes[i];
			aiMesh* sceneMesh = scene->mMeshes[meshIndex];
			m_pModelData->RegisterMesh(sceneMesh, meshIndex);
			m_MeshArrayIndices.push_back(meshIndex);
		}

		for (size_t i = 0; i < modelNode->mNumChildren; i++)
		{
			m_Children.emplace_back(std::make_unique<Node>(m_pModelData, this));
			m_Children.back().get()->ProcessNode(modelNode->mChildren[i], scene);
		}
	}

	DirectX::XMMATRIX Node::ConvertToXMMATRIX(const aiMatrix4x4 & matrix) const
	{
		return DirectX::XMMATRIX(
			matrix.a1, matrix.a2, matrix.a3, matrix.a4,
			matrix.b1, matrix.b2, matrix.b3, matrix.b4,
			matrix.c1, matrix.c2, matrix.c3, matrix.c4,
			matrix.d1, matrix.d2, matrix.d3, matrix.d4
		);
	}
}
