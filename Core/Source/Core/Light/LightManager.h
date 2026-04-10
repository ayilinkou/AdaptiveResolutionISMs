#pragma once

#include <unordered_set>
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
		DirectX::XMMATRIX ViewProj;
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

		static std::unordered_set<Light*>& GetLights() { return s_Lights; }
		static std::vector<PointLight*>& GetPointLights() { return s_PointLights; }
		static std::vector<SpotLight*>& GetSpotLights() { return s_SpotLights; }
		static std::vector<DirectionalLight*>& GetDirectionalLights() { return s_DirLights; }
		static const LightBuffer& GetLightBufferData() { return s_LightBuffer; }
		static Microsoft::WRL::ComPtr<ID3D11Buffer> GetLightCBuffer() { return s_LightCBuffer; }
		static float& GetAmbientStrengthRef() { return s_AmbientStrength; }

		static void UpdateLightBufferData();
		static void UpdateLightCBuffer(const DirectX::XMMATRIX& viewProjT, UINT lightIndex);

	private:
		static void CreateBuffer();

	private:
		static std::unordered_set<Light*> s_Lights;
		static std::vector<PointLight*> s_PointLights;
		static std::vector<SpotLight*> s_SpotLights;
		static std::vector<DirectionalLight*> s_DirLights;
		static LightBuffer s_LightBuffer;
		static float s_AmbientStrength;

		static Microsoft::WRL::ComPtr<ID3D11Buffer> s_LightCBuffer;
	};
}