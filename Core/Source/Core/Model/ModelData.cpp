#include <format>
#include <iostream>

#include "assimp/mesh.h"

#include "ModelData.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Renderer/Material.h"
#include "Core/Utility/MyMacros.h"
#include "Core/Utility/Timer.h"
#include "PointCloudConverter/Converter.h"

namespace Core {
	ModelData::ModelData(const std::string& modelPath, const std::string& texturesPath, const std::string& name, std::vector<Material>&& materials, Node* rootNode, UINT meshCount)
		: m_ModelPath(modelPath), m_TexturesRoot(texturesPath), m_Name(name), m_Materials(std::move(materials)), m_RootNode(std::unique_ptr<Node>(rootNode)),
		m_Meshes(meshCount)
	{
		m_Indices = std::make_unique<std::vector<UINT>>();
		m_Vertices = std::make_unique<std::vector<Vertex>>();

		m_PointCloudCount = 0u;
	}

	ModelData::~ModelData()
	{
		ReleaseIndexAndVertexArrays();
	}

	void ModelData::Init()
	{
		ProcessPointCloudVertices();
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
		m_Indices.reset();
		m_Vertices.reset();
		m_PointCloudPoints.reset();
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

		m_PointCloudCount = (UINT)m_PointCloudPoints->size();
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(DirectX::XMFLOAT3);
		desc.ByteWidth = m_PointCloudCount * sizeof(DirectX::XMFLOAT3);

		data.pSysMem = m_PointCloudPoints->data();

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, &data, &m_PointCloudBuffer));
		NAME_D3D_RESOURCE(m_PointCloudBuffer, (m_ModelPath + " point cloud structured buffer").c_str());

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.NumElements = m_PointCloudCount;
		ASSERT_NOT_FAILED(pDevice->CreateShaderResourceView(m_PointCloudBuffer.Get(), &srvDesc, &m_PointCloudSRV));
		NAME_D3D_RESOURCE(m_PointCloudSRV, (m_ModelPath + " point cloud structured buffer SRV").c_str());
	}

	void ModelData::ProcessPointCloudVertices()
	{
		float density = Renderer::Get()->GetPointCloudDensity();
		Timer loadFromFileTimer("Loading point cloud from file");
		m_PointCloudPoints = PointCloudConverter::LoadFromFile(m_ModelPath, density);
		
		if (m_PointCloudPoints.get())
		{
			// loaded from file successfully
			return;
		}
		else
		{
			// not saved to file
			// process vertices and save to file
			std::cout << std::format("Processing point cloud vertices with density {}...", density) << std::endl;
			loadFromFileTimer.InvalidateTimer();

			Timer processTimer("Processing point cloud vertices");
			m_PointCloudPoints = PointCloudConverter::LoadFromBuffers(*m_Indices, *m_Vertices, m_MeshLocalTransformsT, density);
			processTimer.EndTimer();

			PointCloudConverter::SaveToFile(m_ModelPath, *m_PointCloudPoints, density);
		}
	}
}
