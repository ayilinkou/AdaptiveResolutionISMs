#include "LightManager.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Utility/MyMacros.h"

namespace Core {
	std::vector<Light*> LightManager::s_Lights;
	std::vector<PointLight*> LightManager::s_PointLights;
	std::vector<SpotLight*> LightManager::s_SpotLights;
	std::vector<DirectionalLight*> LightManager::s_DirLights;
	LightBuffer LightManager::s_LightBuffer = {};
	Microsoft::WRL::ComPtr<ID3D11Buffer> LightManager::s_LightCBuffer;
	float LightManager::s_AmbientStrength = 0.2f;

	void LightManager::Init()
	{
		DirectionalLight::InitStatics();
		PointLight::InitStatics();
		SpotLight::InitStatics();
		CreateBuffer();
	}

	void LightManager::Shutdown()
	{
		DirectionalLight::ShutdownStatics();
		PointLight::ShutdownStatics();
		SpotLight::ShutdownStatics();
		s_LightCBuffer.Reset();
	}

	void LightManager::RegisterLight(Light* pLight)
	{
		s_Lights.push_back(pLight);

		PointLight* pPointLight = dynamic_cast<PointLight*>(pLight);
		if (pPointLight)
		{
			s_PointLights.push_back(pPointLight);
			assert(s_PointLights.size() <= MAX_POINT_LIGHT_COUNT);
			return;
		}

		SpotLight* pSpotLight = dynamic_cast<SpotLight*>(pLight);
		if (pSpotLight)
		{
			s_SpotLights.push_back(pSpotLight);
			assert(s_SpotLights.size() <= MAX_SPOT_LIGHT_COUNT);
			return;
		}

		DirectionalLight* pDirLight = dynamic_cast<DirectionalLight*>(pLight);
		if (pDirLight)
		{
			s_DirLights.push_back(pDirLight);
			assert(s_DirLights.size() <= MAX_DIRECTIONAL_LIGHT_COUNT);
			return;
		}
	}

	void LightManager::UnregisterLight(Light* pLight)
	{
		auto it = std::find(s_Lights.begin(), s_Lights.end(), pLight);
		if (it != s_Lights.end())
		{
			s_Lights.erase(it);
		}

		PointLight* pPointLight = dynamic_cast<PointLight*>(pLight);
		if (pPointLight)
		{
			auto it = std::find(s_PointLights.begin(), s_PointLights.end(), pPointLight);
			if (it != s_PointLights.end())
			{
				s_PointLights.erase(it);
			}
			return;
		}

		SpotLight* pSpotLight = dynamic_cast<SpotLight*>(pLight);
		if (pSpotLight)
		{
			auto it = std::find(s_SpotLights.begin(), s_SpotLights.end(), pSpotLight);
			if (it != s_SpotLights.end())
			{
				s_SpotLights.erase(it);
			}
			return;
		}

		DirectionalLight* pDirLight = dynamic_cast<DirectionalLight*>(pLight);
		if (pDirLight)
		{
			auto it = std::find(s_DirLights.begin(), s_DirLights.end(), pDirLight);
			if (it != s_DirLights.end())
			{
				s_DirLights.erase(it);
			}
			return;
		}
	}

	void LightManager::UpdateLightBufferData()
	{
		s_LightBuffer = {};
		s_LightBuffer.AmbientStrength = s_AmbientStrength;

		for (PointLight* pPointLight : s_PointLights)
		{
			if (s_LightBuffer.PointLightCount == MAX_POINT_LIGHT_COUNT)
				break;

			if (!pPointLight || !pPointLight->IsActive())
				continue;

			s_LightBuffer.PointLights[s_LightBuffer.PointLightCount] = pPointLight->GetData();
			s_LightBuffer.PointLightCount++;
		}

		for (SpotLight* pSpotLight : s_SpotLights)
		{
			if (s_LightBuffer.SpotLightCount == MAX_SPOT_LIGHT_COUNT)
				break;

			if (!pSpotLight || !pSpotLight->IsActive())
				continue;

			s_LightBuffer.SpotLights[s_LightBuffer.SpotLightCount] = pSpotLight->GetData();
			s_LightBuffer.SpotLights[s_LightBuffer.SpotLightCount].CosInnerAngle = cos(DirectX::XMConvertToRadians(pSpotLight->m_ConeInnerAngle / 2.f));
			s_LightBuffer.SpotLights[s_LightBuffer.SpotLightCount].CosOuterAngle = cos(DirectX::XMConvertToRadians(pSpotLight->m_ConeOuterAngle / 2.f));
			s_LightBuffer.SpotLightCount++;
		}

		for (UINT i = 0u; i < s_DirLights.size(); i++)
		{
			if (s_LightBuffer.DirectionalLightCount == MAX_DIRECTIONAL_LIGHT_COUNT)
				break;

			DirectionalLight* pDirLight = s_DirLights[i];
			if (!pDirLight || !pDirLight->IsActive())
				continue;

			s_LightBuffer.DirectionalLights[s_LightBuffer.DirectionalLightCount] = pDirLight->GetData();
			s_LightBuffer.DirectionalLightCount++;
		}
	}

	void LightManager::UpdateLightCBuffer(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, UINT lightIndex)
	{
		HRESULT hResult;
		ID3D11DeviceContext* pContext = Renderer::Get()->GetContext().Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};

		ASSERT_NOT_FAILED(pContext->Map(s_LightCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource));
		LightCBuffer* dataPtr = (LightCBuffer*)mappedResource.pData;
		dataPtr->View = viewT;
		dataPtr->Proj = projT;
		dataPtr->LightIndex = lightIndex;
		pContext->Unmap(s_LightCBuffer.Get(), 0u);
	}

	void LightManager::CreateBuffer()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Renderer::Get()->GetDevice().Get();
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(LightCBuffer);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ASSERT_NOT_FAILED(pDevice->CreateBuffer(&desc, nullptr, &s_LightCBuffer));
		NAME_D3D_RESOURCE(s_LightCBuffer, "Light constant buffer");
	}
}