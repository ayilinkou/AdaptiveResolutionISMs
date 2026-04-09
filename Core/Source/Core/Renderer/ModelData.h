#pragma once

#include <string>
#include <vector>
#include <memory>

#include "d3d11.h"
#include "wrl.h"

#include "Material.h"
#include "Texture.h"
#include "Vertex.h"
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
		ModelData(const std::string& modelPath, const std::string& texturesPath, std::vector<Material>&& materials, Node* rootNode, UINT meshCount);
		ModelData(const ModelData&) = delete;
		ModelData& operator=(const ModelData&) = delete;
		ModelData(ModelData&&) noexcept = default;
		ModelData& operator=(ModelData&&) noexcept = default;
		~ModelData();

		void TestDraw();

	private:
		void RegisterMesh(aiMesh* mesh, UINT meshIndex);
		void ReleaseIndexAndVertexArrays();

		void CreateBuffers();

	private:
		ComPtr<ID3D11Buffer> m_IndexBuffer;
		ComPtr<ID3D11Buffer> m_VertexBuffer;

		std::vector<Material> m_Materials;
		std::vector<Mesh> m_Meshes;

		const std::unique_ptr<Node> m_RootNode;
		const std::string m_ModelPath;
		const std::string m_TexturesRoot;

		std::vector<UINT>* m_Indices = nullptr;
		std::vector<Vertex>* m_Vertices = nullptr;
	};
}
