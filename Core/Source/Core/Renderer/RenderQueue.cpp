#include "RenderQueue.h"
#include "Core/Model/Model.h"
#include "Core/Model/ModelData.h"
#include "Renderer.h"
#include "Core/Utility/MyMacros.h"
#include "Core/Model/ModelSystem.h"
#include "Core/Light/LightManager.h"

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

	void RenderQueue::RenderMainPass()
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pRenderer->BindForModelDraws();
		pContext->OMSetRenderTargets(1u, pRenderer->GetBackBufferRTV().GetAddressOf(), pRenderer->GetDSV().Get());
		pRenderer->SetBackBufferViewport();
		RenderPass(RenderPassType::Main);

		ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		pContext->VSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
		pContext->PSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
	}

	void RenderQueue::RenderShadowPass()
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pContext->VSSetConstantBuffers(3u, 1u, LightManager::GetLightCBuffer().GetAddressOf());

		const auto& dirLights = LightManager::GetDirectionalLights();
		if (!dirLights.empty())
		{
			pRenderer->BindForDSVShadowPass();
			pContext->RSSetViewports(1u, &DirectionalLight::GetShadowMapViewport());
			const auto& dirDSVs = DirectionalLight::GetDSVs();

			UINT dirLightCount = min((UINT)dirLights.size(), MAX_DIRECTIONAL_LIGHT_COUNT);
			for (UINT i = 0u; i < dirLightCount; i++)
			{
				DirectionalLight* dirLight = dirLights[i];
				if (!dirLight->IsActive())
					continue;
				
				LightManager::UpdateLightCBuffer(dirLight->GetData().ViewProj, i);
			
				pContext->ClearDepthStencilView(dirDSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
				pContext->OMSetRenderTargets(0u, nullptr, dirDSVs[i].Get());

				RenderPass(RenderPassType::Shadow);
			}
		}

		const auto& spotLights = LightManager::GetSpotLights();
		if (!spotLights.empty())
		{
			pRenderer->BindForDSVShadowPass();
			pContext->RSSetViewports(1u, &SpotLight::GetShadowMapViewport());
			const auto& dirDSVs = SpotLight::GetDSVs();

			UINT spotLightCount = min((UINT)spotLights.size(), MAX_SPOT_LIGHT_COUNT);
			for (UINT i = 0u; i < spotLightCount; i++)
			{
				SpotLight* spotLight = spotLights[i];
				if (!spotLight->IsActive())
					continue;

				LightManager::UpdateLightCBuffer(spotLight->GetData().ViewProj, i);

				pContext->ClearDepthStencilView(dirDSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
				pContext->OMSetRenderTargets(0u, nullptr, dirDSVs[i].Get());

				RenderPass(RenderPassType::Shadow);
			}
		}

		const auto& pointLights = LightManager::GetPointLights();
		if (!pointLights.empty())
		{
			pRenderer->BindForPointShadowPass();
			pContext->PSSetConstantBuffers(1u, 1u, LightManager::GetLightCBuffer().GetAddressOf());
			pContext->RSSetViewports(1u, &PointLight::GetShadowMapViewport());
			const auto& pointRTVs = PointLight::GetRTVs();
			const auto& pointDSV = PointLight::GetDSV();

			float RTVClearColor[4] = { 1.f, 1.f, 1.f, 1.f };
			UINT pointLightCount = min((UINT)pointLights.size(), MAX_POINT_LIGHT_COUNT);
			UINT activePointLightIndex = 0u;
			for (UINT pLightIndex = 0u; pLightIndex < pointLightCount; pLightIndex++)
			{	
				PointLight* pLight = pointLights[pLightIndex];
				if (!pLight->IsActive())
					continue;

				const std::array<DirectX::XMMATRIX, 6> viewProjectionsT = pLight->GetViewProjectionsT();
			
				// for each face
				for (UINT face = 0u; face < 6u; face++)
				{
					Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV = pointRTVs[activePointLightIndex * 6 + face];
				
					pContext->ClearRenderTargetView(RTV.Get(), reinterpret_cast<float*>(&RTVClearColor));
					pContext->ClearDepthStencilView(pointDSV.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);

					LightManager::UpdateLightCBuffer(viewProjectionsT[face], pLightIndex);
					pContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), pointDSV.Get());

					RenderPass(RenderPassType::Shadow);
				}
				activePointLightIndex++;
			}
		}

		ID3D11ShaderResourceView* nullSRVs[] = {nullptr, nullptr};
		pContext->PSSetShaderResources(2u, _countof(nullSRVs), nullSRVs);
		pContext->OMSetRenderTargets(0u, nullptr, nullptr);
	}

	void Core::RenderQueue::RenderPass(RenderPassType passType)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();

		ID3D11Buffer* pCBuffers[2] = { m_ModelLocalCBuffer.Get(), m_ModelWorldCBuffer.Get() };
		pContext->VSSetConstantBuffers(1u, 2u, pCBuffers);

		if (passType == RenderPassType::Main)
		{
			ID3D11ShaderResourceView* shadowMapSRVs[3] = { DirectionalLight::GetShadowMapsSRV().Get(), SpotLight::GetShadowMapsSRV().Get(),
				PointLight::GetShadowMapsSRV().Get() };
			pContext->PSSetShaderResources(2u, _countof(shadowMapSRVs), shadowMapSRVs);
		}

		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			pModelData->BindModelBuffers();

			UINT modelWorldCount = (UINT)modelWorldTransformsT.size();
			assert(modelWorldCount <= MAX_MODEL_WORLD_COUNT);
			UpdateWorldCBuffer(modelWorldTransformsT);

			for (const Mesh& mesh : pModelData->GetMeshes())
			{
				Material* pMat = mesh.GetMaterial();

				if (passType == RenderPassType::Shadow && !pMat->IsOpaque())
					continue;

				if (passType == RenderPassType::Main)
				{
					// bind material resources and buffers
					ID3D11ShaderResourceView* SRVs[2] = { pMat->GetAlbedoSRV(), pMat->GetSpecularSRV() };
					pContext->PSSetShaderResources(0u, _countof(SRVs), SRVs);
					pContext->PSSetConstantBuffers(1u, 1u, pMat->GetCBuffer().GetAddressOf());
				}

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
