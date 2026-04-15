#include <utility>
#include <algorithm>
#include <numeric>

#include "RenderQueue.h"
#include "Renderer.h"
#include "Core/Model/Model.h"
#include "Core/Model/ModelData.h"
#include "Core/Model/ModelSystem.h"
#include "Core/Utility/MyMacros.h"
#include "Core/Utility/Constants.h"
#include "Core/Light/LightManager.h"

namespace Core {
	float RenderQueue::s_ISMCoverageThreshold = 1.f;
	float RenderQueue::s_ISMSplatWorldRadius = 0.05f;
	float RenderQueue::s_LowISMCoverageThreshold = 1.f;
	float RenderQueue::s_LowISMSplatWorldRadius = 0.05f;

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

		desc = {};
		desc.Compute.Filepath = "../Core/Source/Core/Shader/Shaders/ISMTransferCS.hlsl";
		m_IsmTransferShaderProgram = std::make_unique<ShaderProgram>(desc);

		desc = {};
		desc.Compute.Filepath = "../Core/Source/Core/Shader/Shaders/ISMPullPushCS.hlsl";
		desc.Compute.Entry = "Pull";
		m_IsmPullShaderProgram = std::make_unique<ShaderProgram>(desc);

		desc = {};
		desc.Compute.Filepath = "../Core/Source/Core/Shader/Shaders/ISMPullPushCS.hlsl";
		desc.Compute.Entry = "Push";
		m_IsmPushShaderProgram = std::make_unique<ShaderProgram>(desc);

		desc = {};
		desc.Compute.Filepath = "../Core/Source/Core/Shader/Shaders/ISMRankingCS.hlsl";
		m_IsmRankingShaderProgram = std::make_unique<ShaderProgram>(desc);
	}
	
	void RenderQueue::Add(Model* pModel)
	{
		m_ModelWorldTransformsMapT[pModel->GetModelData()].push_back(DirectX::XMMatrixTranspose(pModel->GetTransform().GetWorldMatrix()));
	}

	void RenderQueue::DirShadowPass(const std::vector<DirectionalLight*>& dirLights)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pContext->VSSetConstantBuffers(3u, 1u, LightManager::GetLightCBuffer().GetAddressOf());

		if (!dirLights.empty())
		{
			pRenderer->BindForDSVShadowPass();
			pContext->RSSetViewports(1u, &DirectionalLight::GetShadowMapViewport());
			const auto& dirDSVs = DirectionalLight::GetDSVs();
			UINT activeDirLightCount = 0u;
			UINT dirLightCount = (UINT)dirLights.size();
			for (UINT i = 0u; i < dirLightCount; i++)
			{
				if (activeDirLightCount >= MAX_DIRECTIONAL_LIGHT_COUNT)
					break;

				DirectionalLight* dirLight = dirLights[i];
				if (!dirLight->IsActive())
					continue;

				LightManager::UpdateLightCBuffer(dirLight->GetViewT(), dirLight->GetProjT(), activeDirLightCount);

				pContext->ClearDepthStencilView(dirDSVs[activeDirLightCount].Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
				pContext->OMSetRenderTargets(0u, nullptr, dirDSVs[activeDirLightCount].Get());

				ShadowMapPassSingle();
				activeDirLightCount++;
			}
		}
	}

	void RenderQueue::SpotShadowPass(const std::vector<SpotLight*>& spotLights)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pContext->VSSetConstantBuffers(3u, 1u, LightManager::GetLightCBuffer().GetAddressOf());

		if (!spotLights.empty())
		{
			pRenderer->BindForDSVShadowPass();
			pContext->RSSetViewports(1u, &SpotLight::GetShadowMapViewport());
			const auto& spotDSVs = SpotLight::GetShadowMapDSVs();
			UINT activeSpotLightCount = 0u;
			for (SpotLight* spotLight : spotLights)
			{
				if (activeSpotLightCount >= MAX_SPOT_LIGHT_COUNT)
					break;
				
				if (!spotLight->IsActive())
					continue;
				
				UINT index = spotLight->GetLightBufferID();
				LightManager::UpdateLightCBuffer(spotLight->GetViewT(), spotLight->GetProjT(), index);

				pContext->ClearDepthStencilView(spotDSVs[index].Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
				pContext->OMSetRenderTargets(0u, nullptr, spotDSVs[index].Get());

				ShadowMapPassSingle();

				spotLight->SetShadowType(ShadowType::ShadowMap);
				spotLight->SetShadowMapRes(SpotLight::GetShadowMapRes());
				activeSpotLightCount++;
			}
		}
	}

	void RenderQueue::PointShadowPass(const std::vector<PointLight*>& pointLights)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pContext->VSSetConstantBuffers(3u, 1u, LightManager::GetLightCBuffer().GetAddressOf());
		
		if (!pointLights.empty())
		{
			pRenderer->BindForPointLightShadowPass();
			pContext->PSSetConstantBuffers(1u, 1u, LightManager::GetLightCBuffer().GetAddressOf());
			pContext->RSSetViewports(1u, &PointLight::GetShadowMapViewport());
			const auto& pointRTVs = PointLight::GetRTVs();
			const auto& pointDSV = PointLight::GetDSV();

			float RTVClearColor[4] = { 1.f, 1.f, 1.f, 1.f };
			UINT pointLightCount = (UINT)pointLights.size();
			UINT activePointLightCount = 0u;
			for (UINT pLightIndex = 0u; pLightIndex < pointLightCount; pLightIndex++)
			{
				if (activePointLightCount >= MAX_POINT_LIGHT_COUNT)
					break;

				PointLight* pLight = pointLights[pLightIndex];
				if (!pLight->IsActive())
					continue;

				const std::array<DirectX::XMMATRIX, 6> viewsT = pLight->GetViewsT();

				// for each face
				for (UINT face = 0u; face < 6u; face++)
				{
					Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV = pointRTVs[activePointLightCount * 6 + face];

					pContext->ClearRenderTargetView(RTV.Get(), reinterpret_cast<float*>(&RTVClearColor));
					pContext->ClearDepthStencilView(pointDSV.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);

					LightManager::UpdateLightCBuffer(viewsT[face], pLight->GetProjT(), pLightIndex);
					pContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), pointDSV.Get());

					ShadowMapPassSingle();
				}
				activePointLightCount++;
			}
		}
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

	void RenderQueue::RenderGeometryPass()
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pRenderer->BindForGeometryPass();

		ID3D11Buffer* pCBuffers[2] = { m_ModelLocalCBuffer.Get(), m_ModelWorldCBuffer.Get() };
		pContext->VSSetConstantBuffers(1u, 2u, pCBuffers);

		// opaque only
		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			pModelData->BindModelBuffers();
			UINT modelWorldCount = (UINT)modelWorldTransformsT.size();
			assert(modelWorldCount <= MAX_MODEL_WORLD_COUNT);
			UpdateWorldCBuffer(modelWorldTransformsT);

			for (const Mesh& mesh : pModelData->GetMeshes())
			{
				Material* pMat = mesh.GetMaterial();
				if (!pMat->IsOpaque())
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

		ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		pContext->VSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
		pContext->PSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
		ID3D11RenderTargetView* nullRTVs[] = { nullptr, nullptr, nullptr, nullptr };
		pContext->OMSetRenderTargets(4u, nullRTVs, nullptr);
	}

	void RenderQueue::RenderShadowPass(ShadowMethod shadowMethod)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		
		switch (shadowMethod)
		{
			case ShadowMethod::ShadowMap:
			{
				ShadowMapPass();
				break;
			}
			case ShadowMethod::StaticISM:
			{
				StaticISMPass(LightManager::GetSpotLights().Data());
				break;
			}
			case ShadowMethod::AdaptiveISM:
			{
				AdaptiveISMPass();
				break;
			}
			default:
			{
				assert(false);
			}
		}

		ID3D11ShaderResourceView* nullSRVs[] = {nullptr, nullptr, nullptr, nullptr};
		pContext->PSSetShaderResources(1u, _countof(nullSRVs), nullSRVs);
		pContext->OMSetRenderTargets(0u, nullptr, nullptr);
	}

	void RenderQueue::RenderLightingPass(ShadowMethod shadowMethod)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pRenderer->BindForLightingPass();

		ID3D11Buffer* pCBuffers[2] = { m_ModelLocalCBuffer.Get(), m_ModelWorldCBuffer.Get() };
		pContext->VSSetConstantBuffers(1u, 2u, pCBuffers);

		ID3D11ShaderResourceView* shadowSRVs[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
		switch (shadowMethod)
		{
		case Core::ShadowMethod::ShadowMap:
		{
			shadowSRVs[0] = DirectionalLight::GetShadowMapsSRV().Get();
			shadowSRVs[1] = SpotLight::GetShadowMapsSRV().Get();
			shadowSRVs[2] = PointLight::GetShadowMapsSRV().Get();
			break;
		}
		case Core::ShadowMethod::StaticISM:
		{
			shadowSRVs[0] = DirectionalLight::GetShadowMapsSRV().Get();
			shadowSRVs[1] = SpotLight::GetShadowMapsSRV().Get();
			shadowSRVs[2] = PointLight::GetShadowMapsSRV().Get();
			shadowSRVs[3] = SpotLight::GetISMMipSRVs()[0].Get();
			break;
		}
		case Core::ShadowMethod::AdaptiveISM:
		{
			shadowSRVs[0] = DirectionalLight::GetShadowMapsSRV().Get();
			shadowSRVs[1] = SpotLight::GetShadowMapsSRV().Get();
			shadowSRVs[2] = PointLight::GetShadowMapsSRV().Get();
			shadowSRVs[3] = SpotLight::GetISMMipSRVs()[0].Get();
			shadowSRVs[4] = SpotLight::GetLowISMMipSRVs()[0].Get();
			break;
		}
		default:
			break;
		}

		pContext->PSSetShaderResources(4u, _countof(shadowSRVs), shadowSRVs);

		pContext->DrawIndexed(6u, 0u, 0);

		ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		pContext->PSSetShaderResources(0u, _countof(nullSRVs), nullSRVs);
	}

	void RenderQueue::ShadowMapPassSingle()
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();

		ID3D11Buffer* pCBuffers[2] = { m_ModelLocalCBuffer.Get(), m_ModelWorldCBuffer.Get() };
		pContext->VSSetConstantBuffers(1u, 2u, pCBuffers);

		// opaque
		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			pModelData->BindModelBuffers();

			UINT modelWorldCount = (UINT)modelWorldTransformsT.size();
			assert(modelWorldCount <= MAX_MODEL_WORLD_COUNT);
			UpdateWorldCBuffer(modelWorldTransformsT);

			for (const Mesh& mesh : pModelData->GetMeshes())
			{
				Material* pMat = mesh.GetMaterial();
				if (!pMat->IsOpaque())
					continue;

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
		DirShadowPass(LightManager::GetDirectionalLights().Data());
		SpotShadowPass(LightManager::GetSpotLights().Data());
		PointShadowPass(LightManager::GetPointLights().Data());
	}

	void RenderQueue::StaticISMPass(const std::vector<SpotLight*>& spotLights)
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };
		ID3D11ShaderResourceView* nullSRVs[] = { nullptr };

		std::vector<SpotLight*> activeSpotLights;

		if (!spotLights.empty())
		{
			SplatISMs(spotLights, activeSpotLights, ShadowType::ISM);
		
			if (!activeSpotLights.empty())
			{
				pContext->CSSetConstantBuffers(1u, 1u, m_ISMPullPushCBuffer.GetAddressOf());
				for (UINT i = 0u; i < (UINT)activeSpotLights.size(); i++)
				{
					UINT index = activeSpotLights[i]->GetLightBufferID();
					DispatchISMPull(SpotLight::GetISMRes(), index, SpotLight::GetISM_UAVs()[index], SpotLight::GetISMMipSRVs());
				}

				for (UINT i = 0u; i < (UINT)activeSpotLights.size(); i++)
				{
					UINT index = activeSpotLights[i]->GetLightBufferID();
					DispatchISMPush(SpotLight::GetISMRes(), index, SpotLight::GetISM_UAVs()[index], SpotLight::GetISMMipSRVs());
				}

				DispatchISMRanking(SpotLight::GetISMMipSRVs()[0]);

				// read rankings
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				pContext->CopyResource(m_LightScoresStagingBuffer.Get(), m_LightScoresBuffer.Get());

				pContext->Map(m_LightScoresStagingBuffer.Get(), 0u, D3D11_MAP_READ, 0u, &mapped);
				std::vector<std::pair<UINT, UINT>> lightRanking; // light id, weight
				UINT* rankings = (UINT*)mapped.pData;
				for (UINT i = 0; i < MAX_SPOT_LIGHT_COUNT; i++)
				{
					lightRanking.emplace_back(std::make_pair(i, rankings[i]));
				}
				pContext->Unmap(m_LightScoresStagingBuffer.Get(), 0u);

				// sort indices by light ranking
				std::vector<int> indices(MAX_SPOT_LIGHT_COUNT);
				std::iota(indices.begin(), indices.end(), 0);
				std::sort(indices.begin(), indices.end(),
					[&](const auto a, const auto b)
					{
						return lightRanking[a].second > lightRanking[b].second;
					}
				);

				// sort lights by highest to lowest score
				std::vector<SpotLight*> sortedActiveSpotLights(MAX_SPOT_LIGHT_COUNT, nullptr);
				for (size_t i = 0; i < min(indices.size(), activeSpotLights.size()); i++)
				{
					sortedActiveSpotLights[i] = LightManager::GetActiveSpotLights()[indices[i]];
				}

				// resize to include only top N
				UINT actualSMCount = (UINT)min(activeSpotLights.size(), m_SMCount);
				sortedActiveSpotLights.resize(actualSMCount);

				// set top N to use full res shadow maps
				for (SpotLight* pSpotLight : sortedActiveSpotLights)
				{
					pSpotLight->SetShadowType(ShadowType::ShadowMap);
				}

				// make shadow maps for top N
				SpotShadowPass(sortedActiveSpotLights);

				// update light buffer to use correct shadow type
				pRenderer->UpdateLightsBuffer();
			}
		}

		// Directional and point lights will still use full res shadow maps
		DirShadowPass(LightManager::GetDirectionalLights().Data());
		PointShadowPass(LightManager::GetPointLights().Data());

		pContext->CSSetShaderResources(0u, 1u, nullSRVs);
		pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
	}

	void RenderQueue::AdaptiveISMPass()
	{
		Renderer* pRenderer = Core::Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };
		ID3D11ShaderResourceView* nullSRVs[] = { nullptr };

		const std::vector<SpotLight*>& spotLights = LightManager::GetSpotLights().Data();

		if (!spotLights.empty())
		{
			std::vector<SpotLight*> activeSpotLights;
			SplatISMs(spotLights, activeSpotLights, ShadowType::LowISM);

			if (!activeSpotLights.empty())
			{
				pContext->CSSetConstantBuffers(1u, 1u, m_ISMPullPushCBuffer.GetAddressOf());
				for (UINT i = 0u; i < (UINT)activeSpotLights.size(); i++)
				{
					UINT index = activeSpotLights[i]->GetLightBufferID();
					DispatchISMPull(SpotLight::GetLowISMRes(), index, SpotLight::GetLowISM_UAVs()[index], SpotLight::GetLowISMMipSRVs());
				}

				for (UINT i = 0u; i < (UINT)activeSpotLights.size(); i++)
				{
					UINT index = activeSpotLights[i]->GetLightBufferID();
					DispatchISMPush(SpotLight::GetLowISMRes(), index, SpotLight::GetLowISM_UAVs()[index], SpotLight::GetLowISMMipSRVs());
				}

				DispatchISMRanking(SpotLight::GetLowISMMipSRVs()[0]);

				// read rankings
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				pContext->CopyResource(m_LightScoresStagingBuffer.Get(), m_LightScoresBuffer.Get());

				pContext->Map(m_LightScoresStagingBuffer.Get(), 0u, D3D11_MAP_READ, 0u, &mapped);
				std::vector<std::pair<UINT, UINT>> lightRanking; // light id, weight
				UINT* rankings = (UINT*)mapped.pData;
				for (UINT i = 0; i < MAX_SPOT_LIGHT_COUNT; i++)
				{
					lightRanking.emplace_back(std::make_pair(i, rankings[i]));
				}
				pContext->Unmap(m_LightScoresStagingBuffer.Get(), 0u);

				// sort indices by light ranking
				std::vector<int> indices(MAX_SPOT_LIGHT_COUNT);
				std::iota(indices.begin(), indices.end(), 0);
				std::sort(indices.begin(), indices.end(),
					[&](const auto a, const auto b)
					{
						return lightRanking[a].second > lightRanking[b].second;
					}
				);

				// sort lights by highest to lowest score
				std::vector<SpotLight*> sortedActiveSpotLights(MAX_SPOT_LIGHT_COUNT, nullptr);
				for (size_t i = 0; i < min(indices.size(), activeSpotLights.size()); i++)
				{
					sortedActiveSpotLights[i] = LightManager::GetActiveSpotLights()[indices[i]];
				}

				// resize to include only top N (full res shadow maps) + M (standard ISMs)
				UINT shadowMapAndISMCount = (UINT)min(activeSpotLights.size(), m_SMCount + m_ISMCount);
				sortedActiveSpotLights.resize(shadowMapAndISMCount);

				// run the remaining lights through the standard ISM pass
				StaticISMPass(sortedActiveSpotLights);
			}
		}

		// Handled in the standard ISM pass if at least one spot light is present
		// Directional and point lights will still use full res shadow maps
		if (spotLights.empty())
		{
			DirShadowPass(LightManager::GetDirectionalLights().Data());
			PointShadowPass(LightManager::GetPointLights().Data());
		}

		pContext->CSSetShaderResources(0u, 1u, nullSRVs);
		pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
	}

	void RenderQueue::SplatISMs(const std::vector<SpotLight*>& spotLights, std::vector<SpotLight*>& activeSpotLights, ShadowType shadowType)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };

		std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>>* pSpotUAVs;
		UINT ISMRes;
		D3D11_VIEWPORT ISMViewport;
		float splatRadius;

		switch (shadowType)
		{
			case Core::ShadowType::ISM:
			{
				pSpotUAVs = &SpotLight::GetISM_UAVs();
				ISMRes = SpotLight::GetISMRes();
				ISMViewport = SpotLight::GetISMViewport();
				splatRadius = s_ISMSplatWorldRadius;
				break;
			}
			case Core::ShadowType::LowISM:
			{
				pSpotUAVs = &SpotLight::GetLowISM_UAVs();
				ISMRes = SpotLight::GetLowISMRes();
				ISMViewport = SpotLight::GetLowISMViewport();
				splatRadius = s_LowISMSplatWorldRadius;
				break;
			}
			default:
			{
				assert(false);
				break;
			}
		}
		
		if (!spotLights.empty())
		{
			pContext->CSSetConstantBuffers(1u, 1u, m_ISMSplatCBuffer.GetAddressOf());
			pContext->RSSetViewports(1u, &ISMViewport);
			for (size_t i = 0u; i < spotLights.size(); i++)
			{
				if (activeSpotLights.size() >= MAX_SPOT_LIGHT_COUNT)
					break;

				SpotLight* pSpotLight = spotLights[i];
				if (!pSpotLight->IsActive())
					continue;

				UINT index = pSpotLight->GetLightBufferID();

				const UINT clearValuesUint[4] = { 0xFFFFFFFF, 0u, 0u, 0u };
				pContext->ClearUnorderedAccessViewUint(m_ISMDepthUAV.Get(), clearValuesUint);
				pContext->CSSetUnorderedAccessViews(0u, 1u, m_ISMDepthUAV.GetAddressOf(), nullptr);
				auto& uavMips = pSpotUAVs->at(index);

				DispatchISMSplat(
					pSpotLight->GetViewT(),
					pSpotLight->GetProjT(),
					ISMRes,
					index,
					pSpotLight->GetNearZ(),
					pSpotLight->GetFarZ(),
					splatRadius
				);

				DispatchISMTransfer(ISMRes, uavMips[0]);

				pSpotLight->SetShadowType(shadowType);
				pSpotLight->SetShadowMapRes(ISMRes);
				activeSpotLights.push_back(pSpotLight);
			}
		}

		pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
	}

	void RenderQueue::DispatchISMSplat(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, const UINT ismRes, const UINT lightIndex,
		const float nearPlane, const float farPlane, const float splatRadius)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		pContext->CSSetShader(m_IsmSplatShaderProgram->GetComputeShader(), nullptr, 0u);

		for (const auto& [pModelData, modelWorldTransformsT] : m_ModelWorldTransformsMapT)
		{
			// point cloud is guaranteed to only contain points for opaque meshes so no need to check here
			UINT pointCount = pModelData->GetPointCloudCount();
			UpdateSplatCBuffer(viewT, projT, pointCount, ismRes, lightIndex, nearPlane, farPlane, splatRadius);
			pContext->CSSetShaderResources(0u, 1u, pModelData->GetPointCloudSRV().GetAddressOf());

			// TODO: assert that all points can be drawn in a single compute dispatch
			const UINT groupsX = (UINT)std::ceil((double)pointCount / (double)ISM_SPLAT_THREADS_X);
			pContext->Dispatch(groupsX, 1u, 1u);
		}
	}

	void RenderQueue::DispatchISMTransfer(const UINT ismRes, const Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>& uavMipZero)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		pContext->CSSetShader(m_IsmTransferShaderProgram->GetComputeShader(), nullptr, 0u);
		pContext->CSSetUnorderedAccessViews(0u, 1u, uavMipZero.GetAddressOf(), nullptr);
		pContext->CSSetShaderResources(0u, 1u, m_ISMDepthSRV.GetAddressOf());

		const UINT groupsX = (ismRes + ISM_PULLPUSH_THREADS_X - 1) / ISM_PULLPUSH_THREADS_X;
		const UINT groupsY = (ismRes + ISM_PULLPUSH_THREADS_Y - 1) / ISM_PULLPUSH_THREADS_Y;
		pContext->Dispatch(groupsX, groupsY, 1u);

		ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
		pContext->CSSetShaderResources(0u, 1u, nullSRVs);
	}

	void RenderQueue::DispatchISMPull(const UINT ismRes, const UINT lightIndex, const std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>& spotUAVMips,
		const std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& spotMipSRVs)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		pContext->CSSetShader(m_IsmPullShaderProgram->GetComputeShader(), nullptr, 0u);

		const UINT pullCycles = (UINT)spotUAVMips.size() - 1u;
		ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };

		for (UINT i = 0; i < pullCycles; i++)
		{
			UINT destMipRes = ismRes / (UINT)pow(2.f, (float)i + 1.f);

			UpdatePullPushCBuffer(lightIndex, destMipRes, ismRes);

			pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
			pContext->CSSetShaderResources(0u, 1u, spotMipSRVs[i].GetAddressOf());
			pContext->CSSetUnorderedAccessViews(0u, 1u, spotUAVMips[i + 1u].GetAddressOf(), nullptr);

			const UINT groupsX = (destMipRes + ISM_PULLPUSH_THREADS_X - 1) / ISM_PULLPUSH_THREADS_X;
			const UINT groupsY = (destMipRes + ISM_PULLPUSH_THREADS_Y - 1) / ISM_PULLPUSH_THREADS_Y;
			pContext->Dispatch(groupsX, groupsY, 1u);
		}
	}

	void RenderQueue::DispatchISMPush(const UINT ismRes, const UINT lightIndex, const std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>& spotUAVMips,
		const std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& spotMipSRVs)
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		pContext->CSSetShader(m_IsmPushShaderProgram->GetComputeShader(), nullptr, 0u);

		const UINT pushCycles = (UINT)spotUAVMips.size() - 1u;
		ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };
		ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
		pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
		pContext->CSSetShaderResources(0u, 1u, nullSRVs);

		for (UINT i = 0; i < pushCycles; i++)
		{
			UINT destMipRes = 1 << (i + 1);

			UpdatePullPushCBuffer(lightIndex, destMipRes, ismRes);
			pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
			pContext->CSSetShaderResources(0u, 1u, spotMipSRVs[pushCycles - i].GetAddressOf());
			pContext->CSSetUnorderedAccessViews(0u, 1u, spotUAVMips[pushCycles - i - 1u].GetAddressOf(), nullptr);

			const UINT groupsX = (destMipRes + ISM_PULLPUSH_THREADS_X - 1) / ISM_PULLPUSH_THREADS_X;
			const UINT groupsY = (destMipRes + ISM_PULLPUSH_THREADS_Y - 1) / ISM_PULLPUSH_THREADS_Y;
			pContext->Dispatch(groupsX, groupsY, 1u);
		}
	}

	void RenderQueue::DispatchISMRanking(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& SRVMipZero)
	{
		Renderer* pRenderer = Renderer::Get();
		ID3D11DeviceContext* pContext = pRenderer->GetContext().Get();
		pRenderer->BindForISMRanking();

		UINT clearValues[4] = {};
		pContext->ClearUnorderedAccessViewUint(m_LightScoresUAV.Get(), clearValues);
		pContext->CSSetUnorderedAccessViews(0u, 1u, m_LightScoresUAV.GetAddressOf(), nullptr);
		pContext->CSSetShaderResources(5u, 1u, SRVMipZero.GetAddressOf());
		pContext->CSSetShader(m_IsmRankingShaderProgram->GetComputeShader(), nullptr, 0u);

		const UINT backBufferWidth = pRenderer->GetBackBufferWidth();
		const UINT backBufferHeight = pRenderer->GetBackBufferHeight();
		const UINT groupsX = (backBufferWidth + ISM_RANKING_THREADS_X - 1) / ISM_RANKING_THREADS_X;
		const UINT groupsY = (backBufferHeight + ISM_RANKING_THREADS_Y - 1) / ISM_RANKING_THREADS_Y;
		pContext->Dispatch(groupsX, groupsY, 1u);

		ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };
		pContext->CSSetShaderResources(0u, 6u, nullSRVs);
		pContext->CSSetUnorderedAccessViews(0u, 1u, nullUAVs, nullptr);
	}

	void RenderQueue::CreateBuffers()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Renderer::Get()->GetDevice().Get();
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.ByteWidth = sizeof(ModelLocalBufferData);

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&bufferDesc, nullptr, &m_ModelLocalCBuffer));
		NAME_D3D_RESOURCE(m_ModelLocalCBuffer, "RenderQueue model local constant buffer");

		bufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * MAX_MODEL_WORLD_COUNT;

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&bufferDesc, nullptr, &m_ModelWorldCBuffer));
		NAME_D3D_RESOURCE(m_ModelWorldCBuffer, "RenderQueue model world constant buffer");

		bufferDesc.ByteWidth = sizeof(SplatBufferData);

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&bufferDesc, nullptr, &m_ISMSplatCBuffer));
		NAME_D3D_RESOURCE(m_ISMSplatCBuffer, "RenderQueue ISM splat constant buffer");

		bufferDesc.ByteWidth = sizeof(PullPushBufferData);

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&bufferDesc, nullptr, &m_ISMPullPushCBuffer));
		NAME_D3D_RESOURCE(m_ISMPullPushCBuffer, "RenderQueue ISM pull push constant buffer");

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = SpotLight::GetISMRes();
		texDesc.Height = SpotLight::GetISMRes();
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_UINT;
		texDesc.ArraySize = 1u;
		texDesc.MipLevels = 1u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> ISMDepthTex;
		ASSERT_NOT_FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &ISMDepthTex));
		NAME_D3D_RESOURCE(ISMDepthTex, "Render queue ISM depth texture");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R32_UINT;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0u;

		ASSERT_NOT_FAILED(pDevice->CreateUnorderedAccessView(ISMDepthTex.Get(), &uavDesc, &m_ISMDepthUAV));
		NAME_D3D_RESOURCE(m_ISMDepthUAV, "Render queue ISM depth UAV");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_UINT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2DArray.MipLevels = 1u;

		ASSERT_NOT_FAILED(pDevice->CreateShaderResourceView(ISMDepthTex.Get(), &srvDesc, &m_ISMDepthSRV));
		NAME_D3D_RESOURCE(m_ISMDepthSRV, "Render queue ISM depth SRV ");

		bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(UINT) * MAX_SPOT_LIGHT_COUNT;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = sizeof(UINT);

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&bufferDesc, nullptr, &m_LightScoresBuffer));
		NAME_D3D_RESOURCE(m_LightScoresBuffer, "Render queue light scores buffer");

		bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_STAGING;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		bufferDesc.ByteWidth = sizeof(UINT) * MAX_SPOT_LIGHT_COUNT;

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&bufferDesc, nullptr, &m_LightScoresStagingBuffer));
		NAME_D3D_RESOURCE(m_LightScoresStagingBuffer, "Render queue light scores staging buffer");

		uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.NumElements = MAX_SPOT_LIGHT_COUNT;

		ASSERT_NOT_FAILED(pDevice->CreateUnorderedAccessView(m_LightScoresBuffer.Get(), &uavDesc, &m_LightScoresUAV));
		NAME_D3D_RESOURCE(m_LightScoresUAV, "Render queue light scores UAV");
	}

	void RenderQueue::UpdateLocalCBuffer(const std::vector<DirectX::XMMATRIX>& modelLocalTransformsT)
	{
		HRESULT hResult;
		ID3D11DeviceContext* pContext = Renderer::Get()->GetContext().Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};

		ASSERT_NOT_FAILED(pContext->Map(m_ModelLocalCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource));
		ModelLocalBufferData* localBufferPtr = (ModelLocalBufferData*)mappedResource.pData;
		localBufferPtr->ModelLocalCount = (UINT)modelLocalTransformsT.size();
		memcpy(&localBufferPtr->ModelLocalTransforms, modelLocalTransformsT.data(), sizeof(DirectX::XMMATRIX) * modelLocalTransformsT.size());
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
		const UINT lightIndex, const float nearPlane, const float farPlane, const float splatRadius)
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
		ptr->NearPlane = nearPlane;
		ptr->FarPlane = farPlane;
		ptr->SplatRadiusWorld = splatRadius;
		pContext->Unmap(m_ISMSplatCBuffer.Get(), 0u);
	}

	void RenderQueue::UpdatePullPushCBuffer(const UINT lightIndex, const UINT destMipRes, const UINT ismRes)
	{
		HRESULT hResult;
		ID3D11DeviceContext* pContext = Renderer::Get()->GetContext().Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};

		ASSERT_NOT_FAILED(pContext->Map(m_ISMPullPushCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource));
		PullPushBufferData* ptr = (PullPushBufferData*)mappedResource.pData;
		ptr->LightIndex = lightIndex;
		ptr->DestMipRes = destMipRes;
		ptr->ShadowRes = ismRes;
		ptr->CoverageThreshold = s_ISMCoverageThreshold;
		pContext->Unmap(m_ISMPullPushCBuffer.Get(), 0u);
	}
}
