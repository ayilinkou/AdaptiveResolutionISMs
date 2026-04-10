#pragma once

#include <vector>

#include "Light.h"
#include "Core/Utility/Constants.h"

namespace Core {
	struct DirectionalLightData
	{
		DirectX::XMFLOAT3 Color = { 1.f, 1.f, 1.f };
		float SpecularPower = 512.f;
		DirectX::XMFLOAT3 Dir = { 0.f, -1.f, 0.f };
		float Intensity = 1.f;
		DirectX::XMMATRIX ViewProj;
	};
	
	class DirectionalLight : public Light
	{
	public:
		DirectionalLight(const DirectX::XMFLOAT3 color, const DirectX::XMFLOAT3 dir);
		~DirectionalLight();

		virtual void RenderControls() override;

		void SetDirection(const DirectX::XMFLOAT3 dir);
		virtual void SetColor(float r, float g, float b) override { m_Data.Color = { r, g, b }; }
		virtual void SetSpecularPower(float power) override { m_Data.SpecularPower = power; }
		virtual void SetIntensity(float intensity) override { m_Data.Intensity = intensity; }

		const DirectionalLightData& GetData() const { return m_Data; }

		static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>>& GetDSVs() { return s_DSVs; }
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowMapsSRV() { return s_ShadowMapsSRV; }
		static const D3D11_VIEWPORT& GetShadowMapViewport() { return s_ShadowMapViewport; }

	private:
		static void InitStatics();
		static void ShutdownStatics();
	
	private:
		DirectionalLightData m_Data;
		
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> s_ShadowMapsSRV;
		static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> s_DSVs;
		static D3D11_VIEWPORT s_ShadowMapViewport;
		static DirectX::XMMATRIX s_Proj;

		friend class LightManager;
	};
}