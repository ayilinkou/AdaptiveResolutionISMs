#include "assimp/mesh.h"

#include "ModelData.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Renderer/Material.h"
#include "Core/Utility/MyMacros.h"

namespace Core {
	ModelData::ModelData(const std::string& modelPath, const std::string& texturesPath, const std::string& name, std::vector<Material>&& materials, Node* rootNode, UINT meshCount)
		: m_ModelPath(modelPath), m_TexturesRoot(texturesPath), m_Name(name), m_Materials(std::move(materials)), m_RootNode(std::unique_ptr<Node>(rootNode)),
		m_Meshes(meshCount)
	{
		m_Indices = new std::vector<UINT>();
		m_Vertices = new std::vector<Vertex>();
	}

	ModelData::~ModelData()
	{
		ReleaseIndexAndVertexArrays();
	}

	void ModelData::Init()
	{
		CreateBuffers();
		ReleaseIndexAndVertexArrays();
	}

	void ModelData::BindModelBuffers()
	{
		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0u };

		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		pContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
		pContext->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), strides, offsets);
	}

	Mesh* ModelData::RegisterMesh(aiMesh* mesh, UINT meshIndex, const DirectX::XMMATRIX& localTransformT)
	{
		if (!m_Meshes[meshIndex].IsValid())
			m_Meshes[meshIndex].Init(this, mesh);

		m_MeshLocalTransformsT[&m_Meshes[meshIndex]].push_back(localTransformT);
		return &m_Meshes[meshIndex];
	}

	void ModelData::RegisterLight(aiLight* pLight, const DirectX::XMMATRIX& accumTransform)
	{
		m_Lights.emplace_back(std::make_pair(*pLight, accumTransform));
	}

	void ModelData::ReleaseIndexAndVertexArrays()
	{
		if (m_Indices)
			delete m_Indices;
		if (m_Vertices)
			delete m_Vertices;

		m_Indices = nullptr;
		m_Vertices = nullptr;
	}

	void ModelData::CreateBuffers()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Core::Renderer::Get()->GetDevice().Get();

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.ByteWidth = (UINT)(m_Indices->size() * sizeof(UINT));

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = m_Indices->data();

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, &data, &m_IndexBuffer));
		NAME_D3D_RESOURCE(m_IndexBuffer, (m_ModelPath + " index buffer").c_str());

		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = (UINT)(m_Vertices->size() * sizeof(Vertex));

		data.pSysMem = m_Vertices->data();

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, &data, &m_VertexBuffer));
		NAME_D3D_RESOURCE(m_VertexBuffer, (m_ModelPath + " vertex buffer").c_str());
	}
}
