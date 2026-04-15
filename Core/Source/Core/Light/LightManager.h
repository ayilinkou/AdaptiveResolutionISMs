#pragma once

#include <vector>

#include "DirectXMath.h"

#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "Core/Utility/Constants.h"
#include "Core/Utility/SwapbackArray.h"

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

		static SwapbackArray<Light*>& GetLights() { return s_Lights; }
		static SwapbackArray<PointLight*>& GetPointLights() { return s_PointLights; }
		static SwapbackArray<SpotLight*>& GetSpotLights() { return s_SpotLights; }
		static SwapbackArray<DirectionalLight*>& GetDirectionalLights() { return s_DirLights; }
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

		static void SetSpotLightBiasShadowMap(float minBias, float maxBias) { s_SpotLightMinBiasShadowMap = minBias; s_SpotLightMaxBiasShadowMap = maxBias; }
		static void SetSpotLightBiasISM(float minBias, float maxBias) { s_SpotLightMinBiasISM = minBias; s_SpotLightMaxBiasISM = maxBias; }
		static void SetSpotLightBiasLowISM(float minBias, float maxBias) { s_SpotLightMinBiasLowISM = minBias; s_SpotLightMaxBiasLowISM = maxBias; }

		static void UpdateSpotLights();
		static void UpdateLightBufferData();
		static void UpdateLightCBuffer(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, UINT lightIndex);

	private:
		static void CreateBuffer();

	private:
		static SwapbackArray<Light*> s_Lights;
		static SwapbackArray<PointLight*> s_PointLights;
		static SwapbackArray<SpotLight*> s_SpotLights;
		static SwapbackArray<DirectionalLight*> s_DirLights;
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