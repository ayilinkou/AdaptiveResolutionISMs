#pragma once

#include <vector>
#include <memory>

#include "DirectXMath.h"

#include "assimp/matrix4x4.h"

struct aiNode;
struct aiScene;

typedef unsigned int UINT;

namespace Core {	
	class ModelData;
	class Mesh;

	class Node
	{
	public:
		Node(ModelData* pModelData, Node* pParentNode)
			: m_pModelData(pModelData), m_ParentNode(pParentNode) {}

		void ProcessNode(aiNode* modelNode, const aiScene* scene, const DirectX::XMMATRIX& parentAccumulatedModelLocal);
		void SetModelData(ModelData* pModelData) { m_pModelData = pModelData; }

	private:
		DirectX::XMMATRIX ConvertToXMMATRIX(const aiMatrix4x4& matrix) const;

	private:
		std::vector<std::unique_ptr<Node>> m_Children;
		std::vector<Mesh*> m_Meshes;
		DirectX::XMMATRIX m_AccumulatedModelLocal;

		Node* m_ParentNode;
		ModelData* m_pModelData;
		std::string m_NodeName;
	};
}
