#include "RenderQueue.h"
#include "Core/Model/Model.h"
#include "Core/Model/ModelData.h"
#include "Renderer.h"
#include "Core/Utility/MyMacros.h"
#include "Core/Model/ModelSystem.h"

namespace Core {
	RenderQueue::RenderQueue()
	{
		Init();
	}

	void RenderQueue::Init()
	{
		CreateBuffers();
	}
	
	void RenderQueue::Add(Model* pModel)
	{
		m_ModelWorldTransformsMapT[pModel->GetModelData()].push_back(DirectX::XMMatrixTranspose(pModel->GetTransform().GetWorldMatrix()));
	}
	
	void RenderQueue::PopulateRenderQueue()
	{
		m_ModelWorldTransformsMapT.clear();

		const std::unordered_map<ModelData*, std::unordered_set<Model*>>& allModels = ModelSystem::GetAllModels();
		for (const auto& [modelData, models] : allModels)
		{
			for (Model* m : models)
			{
				if (m->ShouldRender())
					Add(m);
			}
		}
	}

	void RenderQueue::Render()
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pRenderer->BindForModelDraws();

		ID3D11Buffer* pCBuffers[2] = { m_ModelLocalCBuffer.Get(), m_ModelWorldCBuffer.Get() };
		pContext->VSSetConstantBuffers(1u, 2u, pCBuffers);

		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			pModelData->BindModelBuffers();

			UINT modelWorldCount = (UINT)modelWorldTransformsT.size();
			UpdateWorldCBuffer(modelWorldTransformsT);

			for (const Mesh& mesh : pModelData->GetMeshes())
			{
				Material* pMat = mesh.GetMaterial();

				// bind material resources and buffers
				ID3D11ShaderResourceView* SRVs[2] = { pMat->GetAlbedoSRV(), pMat->GetSpecularSRV() };
				pContext->PSSetShaderResources(0u, 2u, SRVs);
				pContext->PSSetConstantBuffers(1u, 1u, pMat->GetCBuffer().GetAddressOf());

				pRenderer->SetBackFaceCulling(!pMat->IsTwoSided());

				const std::vector<DirectX::XMMATRIX>& meshLocalTransformsT = pModelData->GetMeshLocalTransformsT(const_cast<Mesh*>(&mesh));
				UINT meshLocalCount = (UINT)meshLocalTransformsT.size();
				assert(meshLocalCount <= MAX_MODEL_LOCAL_COUNT);

				UpdateLocalCBuffer(meshLocalTransformsT);

				UINT instanceCount = meshLocalCount * modelWorldCount;
				pContext->DrawIndexedInstanced(mesh.GetIndexCount(), instanceCount, mesh.GetIndexOffset(), 0, 0u);
			}
		}
	}

	void RenderQueue::CreateBuffers()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Renderer::Get()->GetDevice().Get();
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.ByteWidth = sizeof(ModelLocalBufferData);

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, nullptr, &m_ModelLocalCBuffer));
		NAME_D3D_RESOURCE(m_ModelLocalCBuffer, "RenderQueue model local constant buffer");

		desc.ByteWidth = sizeof(DirectX::XMMATRIX) * MAX_MODEL_WORLD_COUNT;

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, nullptr, &m_ModelWorldCBuffer));
		NAME_D3D_RESOURCE(m_ModelWorldCBuffer, "RenderQueue model world constant buffer");
	}

	void RenderQueue::UpdateLocalCBuffer(const std::vector<DirectX::XMMATRIX>& modelLocalTransformsT)
	{
		HRESULT hResult;
		ID3D11DeviceContext* pContext = Renderer::Get()->GetContext().Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};

		ASSERT_NOT_FAILED(pContext->Map(m_ModelLocalCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource));
		ModelLocalBufferData* localBufferPtr = (ModelLocalBufferData*)mappedResource.pData;
		localBufferPtr->ModelLocalCount = (UINT)modelLocalTransformsT.size();
		memcpy(&localBufferPtr->modelLocalTransforms, modelLocalTransformsT.data(), sizeof(DirectX::XMMATRIX) * modelLocalTransformsT.size());
		pContext->Unmap(m_ModelLocalCBuffer.Get(), 0u);
	}

	void RenderQueue::UpdateWorldCBuffer(const std::vector<DirectX::XMMATRIX>& modelWorldTransformsT)
	{
		HRESULT hResult;
		ID3D11DeviceContext* pContext = Renderer::Get()->GetContext().Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};

		ASSERT_NOT_FAILED(pContext->Map(m_ModelWorldCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource));
		memcpy(mappedResource.pData, modelWorldTransformsT.data(), sizeof(DirectX::XMMATRIX) * modelWorldTransformsT.size());
		pContext->Unmap(m_ModelWorldCBuffer.Get(), 0u);
	}
}
