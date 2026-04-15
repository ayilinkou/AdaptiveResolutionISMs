#pragma once

#include <vector>

#include "DirectXMath.h"

#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "Core/Utility/Constants.h"

typedef unsigned int UINT;

namespace Core {
	class Light;

	struct LightBuffer
	{
		UINT PointLightCount;
		UINT SpotLightCount;
		UINT DirectionalLightCount;
		float AmbientStrength;
		PointLightData PointLights[MAX_POINT_LIGHT_COUNT];
		SpotLightData SpotLights[MAX_SPOT_LIGHT_COUNT];
		DirectionalLightData DirectionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
	};

	struct LightCBuffer
	{
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Proj;
		UINT LightIndex;
		DirectX::XMFLOAT3 Padding;
	};
	
	class LightManager
	{
	public:
		virtual ~LightManager() = 0;

		static void Init();
		static void Shutdown();

		static void RegisterLight(Light* pLight);
		static void UnregisterLight(Light* pLight);

		static std::vector<Light*>& GetLights() { return s_Lights; }
		static std::vector<PointLight*>& GetPointLights() { return s_PointLights; }
		static std::vector<SpotLight*>& GetSpotLights() { return s_SpotLights; }
		static std::vector<DirectionalLight*>& GetDirectionalLights() { return s_DirLights; }
		static std::vector<SpotLight*>& GetActiveSpotLights() { return s_ActiveSpotLights; }
		static const LightBuffer& GetLightBufferData() { return s_LightBuffer; }
		static Microsoft::WRL::ComPtr<ID3D11Buffer> GetLightCBuffer() { return s_LightCBuffer; }
		static float& GetAmbientStrengthRef() { return s_AmbientStrength; }
		static float& GetSpotLightMinBiasShadowMapRef() { return s_SpotLightMinBiasShadowMap; }
		static float& GetSpotLightMaxBiasShadowMapRef() { return s_SpotLightMaxBiasShadowMap; }
		static float& GetSpotLightMinBiasISMRef() { return s_SpotLightMinBiasISM; }
		static float& GetSpotLightMaxBiasISMRef() { return s_SpotLightMaxBiasISM; }
		static float& GetSpotLightMinBiasLowISMRef() { return s_SpotLightMinBiasLowISM; }
		static float& GetSpotLightMaxBiasLowISMRef() { return s_SpotLightMaxBiasLowISM; }

		static void UpdateSpotLights();
		static void UpdateLightBufferData();
		static void UpdateLightCBuffer(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, UINT lightIndex);

	private:
		static void CreateBuffer();

	private:
		static std::vector<Light*> s_Lights;
		static std::vector<PointLight*> s_PointLights;
		static std::vector<SpotLight*> s_SpotLights;
		static std::vector<DirectionalLight*> s_DirLights;
		static LightBuffer s_LightBuffer;
		static float s_AmbientStrength;
		static float s_SpotLightMinBiasShadowMap;
		static float s_SpotLightMaxBiasShadowMap;
		static float s_SpotLightMinBiasISM;
		static float s_SpotLightMaxBiasISM;
		static float s_SpotLightMinBiasLowISM;
		static float s_SpotLightMaxBiasLowISM;

		static Microsoft::WRL::ComPtr<ID3D11Buffer> s_LightCBuffer;
		static std::vector<SpotLight*> s_ActiveSpotLights;
	};
}