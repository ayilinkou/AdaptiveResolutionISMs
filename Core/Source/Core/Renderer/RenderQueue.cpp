#include "RenderQueue.h"
#include "Renderer.h"
#include "Core/Model/Model.h"
#include "Core/Model/ModelData.h"
#include "Core/Model/ModelSystem.h"
#include "Core/Utility/MyMacros.h"
#include "Core/Utility/Constants.h"
#include "Core/Light/LightManager.h"

namespace Core {
	RenderQueue::RenderQueue()
	{
		Init();
	}

	void RenderQueue::Init()
	{
		CreateBuffers();

		ShaderProgramDesc desc = {};
		desc.Compute.Filepath = "../Core/Source/Core/Shader/Shaders/ISMSplatCS.hlsl";
		m_IsmSplatShaderProgram = std::make_unique<ShaderProgram>(desc);
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

	void RenderQueue::RenderMainPass(ShadowType shadowType)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pContext->OMSetRenderTargets(1u, pRenderer->GetBackBufferRTV().GetAddressOf(), pRenderer->GetDSV().Get());
		pRenderer->SetBackBufferViewport();
		RenderPass(RenderPassType::Main, shadowType);

		ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		pContext->VSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
		pContext->PSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
	}

	void RenderQueue::RenderShadowPass(ShadowType shadowType)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		
		switch (shadowType)
		{
		case ShadowType::ShadowMap:
		{
			ShadowMapPass();
			break;
		}
		case ShadowType::ISM:
		{
			ISMPass();
			break;
		}
		default:
			break;
		}

		ID3D11ShaderResourceView* nullSRVs[] = {nullptr, nullptr, nullptr, nullptr};
		pContext->PSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
		pContext->OMSetRenderTargets(0u, nullptr, nullptr);
	}

	void Core::RenderQueue::RenderPass(RenderPassType passType, ShadowType shadowType)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();

		ID3D11Buffer* pCBuffers[2] = { m_ModelLocalCBuffer.Get(), m_ModelWorldCBuffer.Get() };
		pContext->VSSetConstantBuffers(1u, 2u, pCBuffers);

		if (passType == RenderPassType::Main)
		{
			ID3D11ShaderResourceView* shadowSRVs[3] = { nullptr, nullptr, nullptr };

			switch (shadowType)
			{
			case Core::ShadowType::ShadowMap:
			{
				shadowSRVs[0] = DirectionalLight::GetShadowMapsSRV().Get();
				shadowSRVs[1] = SpotLight::GetShadowMapsSRV().Get();
				shadowSRVs[2] = PointLight::GetShadowMapsSRV().Get();
				break;
			}
			case Core::ShadowType::ISM:
			{
				shadowSRVs[0] = DirectionalLight::GetShadowMapsSRV().Get(); // TODO: do I want to do these too or just spot lights are enough?
				shadowSRVs[1] = SpotLight::GetISM_SRV().Get();
				shadowSRVs[2] = PointLight::GetShadowMapsSRV().Get(); // TODO: do I want to do these too or just spot lights are enough?
				break;
			}
			case Core::ShadowType::None:
				break;
			default:
				break;
			}

			pContext->PSSetShaderResources(3u, _countof(shadowSRVs), shadowSRVs);
		}

		// opaque
		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			if (passType == RenderPassType::Main)
				pRenderer->BindForOpaqueDraws();

			pModelData->BindModelBuffers();

			UINT modelWorldCount = (UINT)modelWorldTransformsT.size();
			assert(modelWorldCount <= MAX_MODEL_WORLD_COUNT);
			UpdateWorldCBuffer(modelWorldTransformsT);

			for (const Mesh& mesh : pModelData->GetMeshes())
			{
				Material* pMat = mesh.GetMaterial();
				if (!pMat->IsOpaque())
					continue;

				if (passType == RenderPassType::Main)
				{
					// bind material resources and buffers
					ID3D11ShaderResourceView* SRVs[3] = { pMat->GetAlbedoSRV(), pMat->GetSpecularSRV(), pMat->GetEmissiveSRV()};
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

		if (passType == RenderPassType::Shadow)
			return;

		// transparent
		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			pRenderer->BindForTransparentDraws();
			pModelData->BindModelBuffers();

			UINT modelWorldCount = (UINT)modelWorldTransformsT.size();
			assert(modelWorldCount <= MAX_MODEL_WORLD_COUNT);
			UpdateWorldCBuffer(modelWorldTransformsT);

			for (const Mesh& mesh : pModelData->GetMeshes())
			{
				Material* pMat = mesh.GetMaterial();
				if (pMat->IsOpaque())
					continue;

				// bind material resources and buffers
				ID3D11ShaderResourceView* SRVs[3] = { pMat->GetAlbedoSRV(), pMat->GetSpecularSRV(), pMat->GetEmissiveSRV() };
				pContext->PSSetShaderResources(0u, _countof(SRVs), SRVs);
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

	void RenderQueue::ShadowMapPass()
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
			UINT activeDirLightCount = 0u;
			UINT dirLightCount = min((UINT)dirLights.size(), MAX_DIRECTIONAL_LIGHT_COUNT);
			for (UINT i = 0u; i < dirLightCount; i++)
			{
				DirectionalLight* dirLight = dirLights[i];
				if (!dirLight->IsActive())
					continue;

				LightManager::UpdateLightCBuffer(dirLight->GetViewT(), dirLight->GetProjT(), activeDirLightCount);

				pContext->ClearDepthStencilView(dirDSVs[activeDirLightCount].Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
				pContext->OMSetRenderTargets(0u, nullptr, dirDSVs[activeDirLightCount].Get());

				RenderPass(RenderPassType::Shadow, ShadowType::ShadowMap);
				activeDirLightCount++;
			}
		}

		const auto& spotLights = LightManager::GetSpotLights();
		if (!spotLights.empty())
		{
			pRenderer->BindForDSVShadowPass();
			pContext->RSSetViewports(1u, &SpotLight::GetShadowMapViewport());
			const auto& spotDSVs = SpotLight::GetShadowMapDSVs();
			UINT activeSpotLightCount = 0u;
			UINT spotLightCount = min((UINT)spotLights.size(), MAX_SPOT_LIGHT_COUNT);
			for (UINT i = 0u; i < spotLightCount; i++)
			{
				SpotLight* spotLight = spotLights[i];
				if (!spotLight->IsActive())
					continue;

				LightManager::UpdateLightCBuffer(spotLight->GetViewT(), spotLight->GetProjT(), activeSpotLightCount);

				pContext->ClearDepthStencilView(spotDSVs[activeSpotLightCount].Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
				pContext->OMSetRenderTargets(0u, nullptr, spotDSVs[activeSpotLightCount].Get());

				RenderPass(RenderPassType::Shadow, ShadowType::ShadowMap);
				activeSpotLightCount++;
			}
		}

		const auto& pointLights = LightManager::GetPointLights();
		if (!pointLights.empty())
		{
			pRenderer->BindForPointLightShadowPass();
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

				const std::array<DirectX::XMMATRIX, 6> viewsT = pLight->GetViewsT();

				// for each face
				for (UINT face = 0u; face < 6u; face++)
				{
					Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV = pointRTVs[activePointLightIndex * 6 + face];

					pContext->ClearRenderTargetView(RTV.Get(), reinterpret_cast<float*>(&RTVClearColor));
					pContext->ClearDepthStencilView(pointDSV.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);

					LightManager::UpdateLightCBuffer(viewsT[face], pLight->GetProjT(), pLightIndex);
					pContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), pointDSV.Get());

					RenderPass(RenderPassType::Shadow, ShadowType::ShadowMap);
				}
				activePointLightIndex++;
			}
		}
	}

	void RenderQueue::ISMPass()
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		pContext->CSSetConstantBuffers(1u, 1u, m_ISMSplatCBuffer.GetAddressOf());
		
		const auto& spotLights = LightManager::GetSpotLights();
		if (!spotLights.empty())
		{
			pContext->RSSetViewports(1u, &SpotLight::GetISMViewport());
			const auto& spotUAVs = SpotLight::GetISM_UAVs();
			UINT activeSpotLightCount = 0u;
			UINT spotLightCount = min((UINT)spotLights.size(), MAX_SPOT_LIGHT_COUNT);
			for (UINT i = 0u; i < spotLightCount; i++)
			{
				SpotLight* spotLight = spotLights[i];
				if (!spotLight->IsActive())
					continue;

				auto& uav = spotUAVs[activeSpotLightCount];

				float clearValues[2] = { 0.f, 0.f };
				pContext->ClearUnorderedAccessViewFloat(uav.Get(), clearValues);
				pContext->CSSetUnorderedAccessViews(0u, 1u, uav.GetAddressOf(), nullptr);
				pContext->CSSetShader(m_IsmSplatShaderProgram->GetComputeShader(), nullptr, 0u);

				DispatchISMSplat(spotLight->GetViewT(), spotLight->GetProjT(), SpotLight::GetISM_Res(), activeSpotLightCount);
				activeSpotLightCount++;
			}
		}

		ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };
		pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
	}

	void RenderQueue::DispatchISMSplat(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, const UINT ismRes, const UINT lightIndex)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		
		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			// point cloud is guaranteed to only contain points for opaque meshes so no need to check here
			UINT pointCount = pModelData->GetPointCloudCount();
			UpdateSplatCBuffer(viewT, projT, pointCount, ismRes, lightIndex);
			pContext->CSSetShaderResources(0u, 1u, pModelData->GetPointCloudSRV().GetAddressOf());

			const UINT groupsX = (ismRes + ISM_SPLAT_THREADS_X - 1) / ISM_SPLAT_THREADS_X;
			const UINT groupsY = (ismRes + ISM_SPLAT_THREADS_Y - 1) / ISM_SPLAT_THREADS_Y;
			pContext->Dispatch(groupsX, groupsY, 1u);
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

		desc.ByteWidth = sizeof(SplatBufferData);

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, nullptr, &m_ISMSplatCBuffer));
		NAME_D3D_RESOURCE(m_ISMSplatCBuffer, "RenderQueue ISM splat constant buffer");
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

	void RenderQueue::UpdateSplatCBuffer(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, UINT pointCount, UINT shadowRes,
		const UINT lightIndex)
	{
		HRESULT hResult;
		ID3D11DeviceContext* pContext = Renderer::Get()->GetContext().Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};

		ASSERT_NOT_FAILED(pContext->Map(m_ISMSplatCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource));
		SplatBufferData* ptr = (SplatBufferData*)mappedResource.pData;
		ptr->LightView = viewT;
		ptr->LightProj = projT;
		ptr->PointCount = pointCount;
		ptr->ShadowRes = shadowRes;
		ptr->LightIndex = lightIndex;
		pContext->Unmap(m_ISMSplatCBuffer.Get(), 0u);
	}
}
