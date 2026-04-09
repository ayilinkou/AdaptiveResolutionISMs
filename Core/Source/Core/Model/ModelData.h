#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>

#include "d3d11.h"
#include "wrl.h"

#include "Core/Renderer/Material.h"
#include "Core/Renderer/Texture.h"
#include "Core/Renderer/Vertex.h"
#include "Node.h"
#include "Mesh.h"

namespace Core {
	namespace Loaders {
		class ModelLoader;
	}

	using namespace Microsoft::WRL;
		
	class ModelData
	{
		friend class Loaders::ModelLoader;
		friend class Model;
		friend class Mesh;
		friend class Node;

	public:
		ModelData(const std::string& modelPath, const std::string& texturesPath, const std::string& name, std::vector<Material>&& materials, Node* rootNode,
			UINT meshCount);
		ModelData(const ModelData&) = delete;
		ModelData& operator=(const ModelData&) = delete;
		ModelData(ModelData&&) noexcept = default;
		ModelData& operator=(ModelData&&) noexcept = default;
		~ModelData();

		void Init();

		void BindModelBuffers();

		const std::vector<Mesh>& GetMeshes() const { return m_Meshes; }
		const std::vector<DirectX::XMMATRIX>& GetMeshLocalTransformsT(Mesh* pMesh) const { return m_MeshLocalTransformsT.at(pMesh); }

	private:
		Mesh* RegisterMesh(aiMesh* mesh, UINT meshIndex, const DirectX::XMMATRIX& localTransform);
		void CreateBuffers();

		void ReleaseIndexAndVertexArrays();

	private:
		ComPtr<ID3D11Buffer> m_IndexBuffer;
		ComPtr<ID3D11Buffer> m_VertexBuffer;

		std::vector<Material> m_Materials;
		std::vector<Mesh> m_Meshes;
		std::unordered_map<Mesh*, std::vector<DirectX::XMMATRIX>> m_MeshLocalTransformsT;

		const std::unique_ptr<Node> m_RootNode; // TODO: I think this isn't needed after the mesh is processed, as long as these are static meshes
		const std::string m_Name;
		const std::string m_ModelPath;
		const std::string m_TexturesRoot;

		std::vector<UINT>* m_Indices = nullptr;
		std::vector<Vertex>* m_Vertices = nullptr;
	};
}
