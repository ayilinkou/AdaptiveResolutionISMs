#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <array>

#include "d3d11.h"
#include "wrl.h"
#include "assimp/light.h"

#include "Core/Renderer/Material.h"
#include "Core/Renderer/Texture.h"
#include "Core/Renderer/Vertex.h"
#include "Node.h"
#include "Mesh.h"

namespace Core {
	namespace Loaders {
		class ModelLoader;
	}
		
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
		UINT GetPointCloudCount() const { return m_PointCloudCount; }
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetPointCloudSRV() const { return m_PointCloudSRV; }

	private:
		Mesh* RegisterMesh(aiMesh* mesh, UINT meshIndex, const DirectX::XMMATRIX& localTransform);
		void RegisterLight(aiLight* pLight, const DirectX::XMMATRIX& accumTransform);
		void CreateBuffers();

		void ProcessPointCloudVertices();

		void ReleaseIndexAndVertexArrays();

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_PointCloudBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_PointCloudSRV;
		UINT m_PointCloudCount;

		std::vector<Material> m_Materials;
		std::vector<Mesh> m_Meshes;
		std::vector<std::pair<aiLight, DirectX::XMMATRIX>> m_Lights;
		std::unordered_map<Mesh*, std::vector<DirectX::XMMATRIX>> m_MeshLocalTransformsT;

		const std::unique_ptr<Node> m_RootNode; // TODO: I think this isn't needed after the mesh is processed, as long as these are static meshes
		const std::string m_Name;
		const std::string m_ModelPath;
		const std::string m_TexturesRoot;

		std::unique_ptr<std::vector<UINT>> m_Indices;
		std::unique_ptr<std::vector<Vertex>> m_Vertices;
		std::unique_ptr<std::vector<DirectX::XMFLOAT3>> m_PointCloudPoints;
	};
}
