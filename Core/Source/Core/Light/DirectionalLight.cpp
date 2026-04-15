#include "imgui.h"

#include "DirectionalLight.h"
#include "LightManager.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Utility/MyMacros.h"

namespace Core {
	const DirectX::XMMATRIX DirectionalLight::s_Proj = DirectX::XMMatrixOrthographicLH(100.f, 100.f, 0.1f, 1000.f);
	const DirectX::XMMATRIX DirectionalLight::s_ProjT = DirectX::XMMatrixTranspose(DirectionalLight::s_Proj);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DirectionalLight::s_ShadowMapsSRV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> DirectionalLight::s_DSVs;
	
	DirectionalLight::DirectionalLight(const DirectX::XMFLOAT3 color, const DirectX::XMFLOAT3 dir)
	{
		m_Data = {};
		m_Data.Color = color;
		m_Data.ShadowMapRes = DirectionalLight::GetShadowMapRes();
		m_Name = "Directional Light";
		m_ViewT = DirectX::XMMatrixIdentity();
		LightManager::RegisterLight(this);
		SetDirection(dir);
	}

	DirectionalLight::~DirectionalLight()
	{
		LightManager::UnregisterLight(this);
	}

	void DirectionalLight::InitStatics()
	{
		HRESULT hResult;
		ID3D11Device* pDevice = Renderer::Get()->GetDevice().Get();

		Microsoft::WRL::ComPtr<ID3D11Texture2D> textureArray;
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = (UINT)s_ShadowMapViewport.Width;
		texDesc.Height = (UINT)s_ShadowMapViewport.Height;
		texDesc.ArraySize = MAX_DIRECTIONAL_LIGHT_COUNT;
		texDesc.MipLevels = 1u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

		ASSERT_NOT_FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &textureArray));
		NAME_D3D_RESOURCE(textureArray, "Directional light shadow maps texture array");

		for (UINT i = 0; i < MAX_DIRECTIONAL_LIGHT_COUNT; i++)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 1u;
			dsvDesc.Texture2DArray.FirstArraySlice = i;

			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
			ASSERT_NOT_FAILED(pDevice->CreateDepthStencilView(textureArray.Get(), &dsvDesc, &dsv));
			NAME_D3D_RESOURCE(dsv, (std::string("Directional light shadow map DSV ") + std::to_string(i)).c_str());
			s_DSVs.push_back(dsv);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1u;
		srvDesc.Texture2DArray.ArraySize = MAX_DIRECTIONAL_LIGHT_COUNT;

		ASSERT_NOT_FAILED(pDevice->CreateShaderResourceView(textureArray.Get(), &srvDesc, &s_ShadowMapsSRV));
		NAME_D3D_RESOURCE(s_ShadowMapsSRV, "Directional light shadow maps SRV");
	}

	void DirectionalLight::ShutdownStatics()
	{
		s_ShadowMapsSRV.Reset();
		s_DSVs.clear();
	}

	void DirectionalLight::RenderControls()
	{
		ImGui::Text("Directional Light");
		ImGui::Checkbox("Active", &m_bActive);
		ImGui::ColorEdit3("Diffuse Color", reinterpret_cast<float*>(&m_Data.Color));

		DirectX::XMFLOAT3 dir = m_Data.Dir;
		if (ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&dir), 0.1f))
		{
			SetDirection(dir);
		}

		ImGui::SliderFloat("Specular Power", &m_Data.SpecularPower, 1.f, 2048.f, "%.f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Intensity", &m_Data.Intensity, 0.f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	}

	void DirectionalLight::SetDirection(const DirectX::XMFLOAT3 dir)
	{
		const DirectX::XMFLOAT3 initialDir = m_Data.Dir;
		
		assert(!(dir.x == 0 && dir.y == 0 && dir.z == 0));
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&dir);
		v = DirectX::XMVector3Normalize(v);
		DirectX::XMStoreFloat3(&m_Data.Dir, v);

		constexpr float distance = -300.f;
		const DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(v, distance);
		const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(lightPos, DirectX::XMVectorZero(), DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f));
		m_ViewT = DirectX::XMMatrixTranspose(view);
		m_Data.ViewProj = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(view, s_Proj));
	}
}
