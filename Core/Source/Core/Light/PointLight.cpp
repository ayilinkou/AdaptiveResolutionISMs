#include "imgui.h"

#include "PointLight.h"
#include "LightManager.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Utility/MyMacros.h"

namespace Core {
	constexpr UINT SHADOW_MAP_RES = 256u;
	D3D11_VIEWPORT PointLight::s_ShadowMapViewport = { 0.f, 0.f, (float)SHADOW_MAP_RES, (float)SHADOW_MAP_RES, 0.f, 1.f };
	std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> PointLight::s_RTVs;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> PointLight::s_DSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> PointLight::s_ShadowMapsSRV;

	PointLight::PointLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 attenuation)
		: m_LocalPosition({ 0.f, 0.f, 0.f })
	{
		m_Data = {};
		m_Data.Color = color;
		m_Data.Attenuation = attenuation;
		m_Name = "Point Light";
		UpdateProj();
		LightManager::RegisterLight(this);
	}

	PointLight::~PointLight()
	{
		LightManager::UnregisterLight(this);
	}

	void PointLight::InitStatics()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Renderer::Get()->GetDevice().Get();

		constexpr UINT totalFaces = MAX_POINT_LIGHT_COUNT * 6u;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> textureCubeArray;
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = (UINT)s_ShadowMapViewport.Width;
		texDesc.Height = (UINT)s_ShadowMapViewport.Height;
		texDesc.ArraySize = totalFaces;
		texDesc.MipLevels = 1u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_FLOAT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		ASSERT_NOT_FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &textureCubeArray));
		NAME_D3D_RESOURCE(textureCubeArray, "Point light shadow maps texture cube array");

		for (UINT i = 0; i < totalFaces; i++)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 1u;
			rtvDesc.Texture2DArray.FirstArraySlice = i;

			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
			ASSERT_NOT_FAILED(pDevice->CreateRenderTargetView(textureCubeArray.Get(), &rtvDesc, &rtv));
			NAME_D3D_RESOURCE(rtv, (std::string("Point light shadow map RTV ") + std::to_string(i / 6u) + " face " + std::to_string(i % 6)).c_str());
			s_RTVs.push_back(rtv);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		srvDesc.TextureCubeArray.MipLevels = 1u;
		srvDesc.TextureCubeArray.NumCubes = MAX_POINT_LIGHT_COUNT;

		ASSERT_NOT_FAILED(pDevice->CreateShaderResourceView(textureCubeArray.Get(), &srvDesc, &s_ShadowMapsSRV));
		NAME_D3D_RESOURCE(s_ShadowMapsSRV, "Point light shadow maps SRV");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> dsvTexture;
		texDesc = {};
		texDesc.Width = (UINT)s_ShadowMapViewport.Width;
		texDesc.Height = (UINT)s_ShadowMapViewport.Height;
		texDesc.ArraySize = 1u;
		texDesc.MipLevels = 1u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ASSERT_NOT_FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &dsvTexture));
		NAME_D3D_RESOURCE(dsvTexture, "Point light shadow map DSV texture");

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		ASSERT_NOT_FAILED(pDevice->CreateDepthStencilView(dsvTexture.Get(), &dsvDesc, &s_DSV));
		NAME_D3D_RESOURCE(s_DSV, "Point light shadow map DSV");
	}

	void PointLight::ShutdownStatics()
	{
		s_ShadowMapsSRV.Reset();
		s_DSV.Reset();
		s_RTVs.clear();
	}

	void PointLight::RenderControls()
	{
		ImGui::Text("Point Light");
		ImGui::Checkbox("Active", &m_bActive);
		ImGui::ColorEdit3("Diffuse Color", reinterpret_cast<float*>(&m_Data.Color));

		if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&m_LocalPosition), 0.1f))
			SetPosition(m_LocalPosition.x, m_LocalPosition.y, m_LocalPosition.z);

		ImGui::DragFloat("Attenuation (quadratic)", &m_Data.Attenuation.x, 0.1f);
		ImGui::DragFloat("Attenuation (linear)", &m_Data.Attenuation.y, 0.1f);
		ImGui::DragFloat("Attenuation (constant)", &m_Data.Attenuation.z, 0.1f);
		ImGui::SliderFloat("Specular Power", &m_Data.SpecularPower, 1.f, 2048.f, "%.f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Intensity", &m_Data.Intensity, 0.f, 10.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		
		if (ImGui::SliderFloat("Radius", &m_Data.Radius, 1.f, 500.f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
			UpdateProj();
	}

	void PointLight::SetPosition(const DirectX::XMFLOAT3 pos)
	{
		SetPosition(pos.x, pos.y, pos.z);
	}

	void PointLight::SetPosition(float x, float y, float z)
	{
		m_LocalPosition = { x, y, z };

		DirectX::XMVECTOR modelV = DirectX::XMVector3TransformCoord(DirectX::XMVectorZero(), m_AccumTransform);
		DirectX::XMVECTOR worldV = DirectX::XMLoadFloat3(&m_LocalPosition);
		DirectX::XMVECTOR sumV = DirectX::XMVectorAdd(modelV, worldV);

		DirectX::XMStoreFloat3(&m_Data.Position, sumV);
	}

	void PointLight::UpdateProj()
	{
		m_Proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.f), 1.f, 0.1f, m_Data.Radius);
	}

	const std::array<DirectX::XMMATRIX, 6> PointLight::GetViewsT() const
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_Data.Position);

		DirectX::XMMATRIX views[6];
		views[0] = DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, DirectX::XMVectorSet( 1,  0,  0, 0)), DirectX::XMVectorSet(0, 1,  0, 0)); // +X
		views[1] = DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, DirectX::XMVectorSet(-1,  0,  0, 0)), DirectX::XMVectorSet(0, 1,  0, 0)); // -X
		views[2] = DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, DirectX::XMVectorSet( 0,  1,  0, 0)), DirectX::XMVectorSet(0, 0, -1, 0)); // +Y
		views[3] = DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, DirectX::XMVectorSet( 0, -1,  0, 0)), DirectX::XMVectorSet(0, 0,  1, 0)); // -Y
		views[4] = DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, DirectX::XMVectorSet( 0,  0,  1, 0)), DirectX::XMVectorSet(0, 1,  0, 0)); // +Z
		views[5] = DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, DirectX::XMVectorSet( 0,  0, -1, 0)), DirectX::XMVectorSet(0, 1,  0, 0)); // -Z

		std::array<DirectX::XMMATRIX, 6> viewsT;
		for (int i = 0; i < 6; i++)
		{
			viewsT[i] = DirectX::XMMatrixTranspose(views[i]);
		}
		return viewsT;
	}
}