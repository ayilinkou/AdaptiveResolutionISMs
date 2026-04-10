#include "imgui.h"

#include "SpotLight.h"
#include "LightManager.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Utility/MyMacros.h"

inline DirectX::XMVECTOR ComputeSafeUpVector(const DirectX::XMVECTOR& dir)
{
	const DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	const DirectX::XMVECTOR backupUp = DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f);

	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(dir, worldUp));

	// if direction is too close to world up/down, use backup
	if (fabsf(dot) > 0.99f)
		return backupUp;
	return worldUp;
}

namespace Core {
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> SpotLight::s_ShadowMapDSVs;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SpotLight::s_ShadowMapsSRV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>> SpotLight::s_ISM_UAVs;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SpotLight::s_ISM_SRV;

	SpotLight::SpotLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 attenuation, DirectX::XMFLOAT3 dir)
		: m_LocalPosition({ 0.f, 0.f, 0.f })
	{
		m_Data.Color = color;
		m_Data.Attenuation = attenuation;
		m_Data.Direction = dir;
		m_Name = "Spot Light";
		UpdateView();
		UpdateProj();
		UpdateViewProj();
		LightManager::RegisterLight(this);
	}

	SpotLight::~SpotLight()
	{
		LightManager::UnregisterLight(this);
	}

	void SpotLight::InitStatics()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Renderer::Get()->GetDevice().Get();

		// shadow maps
		Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowMapTextureArray;
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = (UINT)s_ShadowMapViewport.Width;
		texDesc.Height = (UINT)s_ShadowMapViewport.Height;
		texDesc.ArraySize = MAX_SPOT_LIGHT_COUNT;
		texDesc.MipLevels = 1u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

		ASSERT_NOT_FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &shadowMapTextureArray));
		NAME_D3D_RESOURCE(shadowMapTextureArray, "Spot light shadow maps texture array");

		for (UINT i = 0; i < MAX_SPOT_LIGHT_COUNT; i++)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 1u;
			dsvDesc.Texture2DArray.FirstArraySlice = i;

			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
			ASSERT_NOT_FAILED(pDevice->CreateDepthStencilView(shadowMapTextureArray.Get(), &dsvDesc, &dsv));
			NAME_D3D_RESOURCE(dsv, (std::string("Spot light shadow map DSV ") + std::to_string(i)).c_str());
			s_ShadowMapDSVs.push_back(dsv);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1u;
		srvDesc.Texture2DArray.ArraySize = MAX_SPOT_LIGHT_COUNT;

		ASSERT_NOT_FAILED(pDevice->CreateShaderResourceView(shadowMapTextureArray.Get(), &srvDesc, &s_ShadowMapsSRV));
		NAME_D3D_RESOURCE(s_ShadowMapsSRV, "Spot light shadow maps SRV");

		// ISMs
		UINT ismMipCount = 1u; // TODO: will change this when need pull push algorithm
		Microsoft::WRL::ComPtr<ID3D11Texture2D> ISMTextureArray;
		texDesc.Width = (UINT)s_ISMViewport.Width;
		texDesc.Height = (UINT)s_ISMViewport.Height;
		texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		texDesc.ArraySize = MAX_SPOT_LIGHT_COUNT;
		texDesc.MipLevels = ismMipCount; 
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

		ASSERT_NOT_FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &ISMTextureArray));
		NAME_D3D_RESOURCE(ISMTextureArray, "Spot light ISMs texture array");

		for (UINT i = 0u; i < MAX_SPOT_LIGHT_COUNT; i++)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.ArraySize = 1u;
			uavDesc.Texture2DArray.FirstArraySlice = i;

			Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
			ASSERT_NOT_FAILED(pDevice->CreateUnorderedAccessView(ISMTextureArray.Get(), &uavDesc, &uav));
			NAME_D3D_RESOURCE(uav, (std::string("Spot light ISM UAV ") + std::to_string(i)).c_str());
			s_ISM_UAVs.push_back(uav);
		}

		srvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		srvDesc.Texture2DArray.MipLevels = ismMipCount;
		ASSERT_NOT_FAILED(pDevice->CreateShaderResourceView(ISMTextureArray.Get(), &srvDesc, &s_ISM_SRV));
		NAME_D3D_RESOURCE(s_ISM_SRV, "Spot light ISMs SRV");
	}

	void SpotLight::ShutdownStatics()
	{
		s_ShadowMapsSRV.Reset();
		s_ShadowMapDSVs.clear();
		s_ISM_SRV.Reset();
		s_ISM_UAVs.clear();
	}

	void SpotLight::RenderControls()
	{		
		ImGui::Text("Spot Light");
		ImGui::Checkbox("Active", &m_bActive);
		ImGui::ColorEdit3("Diffuse Color", reinterpret_cast<float*>(&m_Data.Color));

		if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&m_LocalPosition), 0.1f))
			SetPosition(m_LocalPosition.x, m_LocalPosition.y, m_LocalPosition.z);

		DirectX::XMFLOAT3 dir(&m_Data.Direction.x);
		if (ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&dir), 0.01f))
			SetDirection(dir.x, dir.y, dir.z);

		ImGui::DragFloat("Attenuation (quadratic)", &m_Data.Attenuation.x, 0.1f);
		ImGui::DragFloat("Attenuation (linear)", &m_Data.Attenuation.y, 0.1f);
		ImGui::DragFloat("Attenuation (constant)", &m_Data.Attenuation.z, 0.1f);
		ImGui::SliderFloat("Specular Power", &m_Data.SpecularPower, 1.f, 2048.f, "%.f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Intensity", &m_Data.Intensity, 0.f, 50.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::SliderFloat("Radius", &m_Data.Radius, 1.f, 100.f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
			UpdateProj();

		if (ImGui::SliderFloat("Cone Inner Angle", &m_ConeInnerAngle, 0.f, 89.f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
		{
			if (m_ConeInnerAngle > m_ConeOuterAngle)
			{
				m_ConeOuterAngle = m_ConeInnerAngle + 0.01f;
				UpdateProj();
			}
		}

		if (ImGui::SliderFloat("Cone Outer Angle", &m_ConeOuterAngle, 0.f, 89.f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
		{
			if (m_ConeOuterAngle < m_ConeInnerAngle)
				m_ConeInnerAngle = m_ConeOuterAngle - 0.01f;
			UpdateProj();
		}
	}

	void SpotLight::SetPosition(float x, float y, float z)
	{
		m_LocalPosition = { x, y, z };

		DirectX::XMVECTOR modelV = DirectX::XMVector3TransformCoord(DirectX::XMVectorZero(), m_AccumTransform);
		DirectX::XMVECTOR worldV = DirectX::XMLoadFloat3(&m_LocalPosition);
		DirectX::XMVECTOR sumV = DirectX::XMVectorAdd(modelV, worldV);

		DirectX::XMStoreFloat3(&m_Data.Position, sumV);
		UpdateView();
	}

	void SpotLight::SetDirection(float x, float y, float z)
	{
		assert(!(x == 0 && y == 0 && z == 0));
		DirectX::XMVECTOR v = DirectX::XMVectorSet(x, y, z, 0.f);
		v = DirectX::XMVector3Transform(v, m_AccumTransform);
		v = DirectX::XMVector3Normalize(v);
		DirectX::XMStoreFloat3(&m_Data.Direction, v);
		UpdateView();
	}

	void SpotLight::SetAngles(float inner, float outer)
	{
		if (inner > outer)
			outer = inner + 0.01f;
		
		m_ConeInnerAngle = inner;
		m_ConeOuterAngle = outer;
		UpdateProj();
	}

	void SpotLight::UpdateView()
	{
		const DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(m_Data.Position.x, m_Data.Position.y, m_Data.Position.z, 1.f);
		const DirectX::XMVECTOR dir = DirectX::XMVectorSet(m_Data.Direction.x, m_Data.Direction.y, m_Data.Direction.z, 0.f);
		const DirectX::XMVECTOR focus = DirectX::XMVectorAdd(lightPos, dir);

		DirectX::XMVECTOR up = ComputeSafeUpVector(dir);
		const DirectX::XMVECTOR dirCrossUp = DirectX::XMVector3Cross(dir, up);
		DirectX::XMVECTOR lightUp = DirectX::XMVector3Cross(dirCrossUp, dir); // this might be upside down but don't think it really matters
		m_View = DirectX::XMMatrixLookAtLH(lightPos, focus, lightUp);

		UpdateViewProj();
	}

	void SpotLight::UpdateProj()
	{
		m_Proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_ConeOuterAngle), 1.f, 0.1f, m_Data.Radius);
		UpdateViewProj();
	}

	void SpotLight::UpdateViewProj()
	{
		m_Data.ViewProj = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(m_View, m_Proj));
	}
}