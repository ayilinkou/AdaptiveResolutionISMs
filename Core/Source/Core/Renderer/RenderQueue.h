#pragma once

#include <unordered_map>
#include <memory>

#include "d3d11.h"
#include "wrl.h"
#include "DirectXMath.h"

#include "Core/Utility/Constants.h"
#include "Core/Shader/ShaderProgram.h"

namespace Core
{
	class Model;
	class ModelData;
	class DirectionalLight;
	class SpotLight;
	class PointLight;

	enum class ShadowMethod
	{
		ShadowMap,
		StaticISM,
		AdaptiveISM
	};

	enum class ShadowType
	{
		ShadowMap,
		ISM,
		LowISM
	};

	class RenderQueue
	{
	private:
		struct ModelLocalBufferData
		{
			UINT ModelLocalCount;
			DirectX::XMFLOAT3 Padding;
			DirectX::XMMATRIX ModelLocalTransforms[MAX_MODEL_LOCAL_COUNT];
		};

		struct SplatBufferData
		{
			DirectX::XMMATRIX LightView;
			DirectX::XMMATRIX LightProj;
			UINT PointCount;
			UINT ShadowRes;
			UINT LightIndex;
			float NearPlane;
			float FarPlane;
			float SplatRadiusWorld;
			DirectX::XMFLOAT2 Padding;
		};

		struct PullPushBufferData
		{
			UINT LightIndex;
			UINT DestMipRes;
			UINT ShadowRes;
			float CoverageThreshold;
		};

	public:
		RenderQueue();

		void PopulateRenderQueue();
		void RenderGeometryPass();
		void RenderShadowPass(ShadowMethod shadowMethod);
		void RenderLightingPass(ShadowMethod shadowMethod);

		void SetSMCount(const UINT count) { m_SMCount = count; }
		void SetISMCount(const UINT count) { m_ISMCount = count; }

		static float& GetISMCoverageThresholdRef() { return s_ISMCoverageThreshold; }
		static float& GetISMSplatWorldRadiusRef() { return s_ISMSplatWorldRadius; }
		static float& GetLowISMCoverageThresholdRef() { return s_LowISMCoverageThreshold; }
		static float& GetLowISMSplatWorldRadiusRef() { return s_LowISMSplatWorldRadius; }

	private:
		void Init();
		void CreateBuffers();

		void ShadowMapPassSingle();

		void UpdateLocalCBuffer(const std::vector<DirectX::XMMATRIX>& modelLocalTransformsT);
		void UpdateWorldCBuffer(const std::vector<DirectX::XMMATRIX>& modelWorldTransformsT);
		void UpdateSplatCBuffer(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, const UINT pointCount,
			const UINT shadowRes, const UINT lightIndex, const float nearPlane, const float farPlane, const float splatRadius);
		void UpdatePullPushCBuffer(const UINT lightIndex, const UINT destMipRes, const UINT ismRes);
		
		void Add(Model* pModel);

		void DirShadowPass(const std::vector<DirectionalLight*>& dirLights);
		void SpotShadowPass(const std::vector<SpotLight*>& spotLights);
		void SpotShadowPass(const std::vector<std::pair<SpotLight*, UINT>>& spotLights);
		void PointShadowPass(const std::vector<PointLight*>& pointLights);

		void ShadowMapPass();
		void StaticISMPass(const std::vector<SpotLight*>& spotLights);
		void AdaptiveISMPass();

		void SplatISMs(const std::vector<SpotLight*>& spotLights, std::vector<SpotLight*>& activeSpotLights, ShadowType shadowType);
		void DispatchISMSplat(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, const UINT ismRes, const UINT lightIndex,
			const float nearPlane, const float farPlane, const float splatRadius);
		void DispatchISMTransfer(const UINT ismRes, const Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>& uavMipZero);
		void DispatchISMPull(const UINT ismRes, const UINT lightIndex, const std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>& spotUAVMips,
			const std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& spotMipSRVs);
		void DispatchISMPush(const UINT ismRes, const UINT lightIndex, const std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>& spotUAVMips,
			const std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& spotMipSRVs);
		void DispatchISMRanking(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& SRVMipZero);

	private:
		std::unordered_map<ModelData*, std::vector<DirectX::XMMATRIX>> m_ModelWorldTransformsMapT;

		// TODO: structured buffers would probably be better for both of these
		// per model mesh transform (model space)
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ModelLocalCBuffer;

		// per instance transform (world space)
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ModelWorldCBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ISMSplatCBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ISMPullPushCBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_LightScoresBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_LightScoresStagingBuffer;

		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_ISMDepthUAV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ISMDepthSRV;

		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_LightScoresUAV;

		std::unique_ptr<ShaderProgram> m_IsmSplatShaderProgram;
		std::unique_ptr<ShaderProgram> m_IsmTransferShaderProgram;
		std::unique_ptr<ShaderProgram> m_IsmPullShaderProgram;
		std::unique_ptr<ShaderProgram> m_IsmPushShaderProgram;
		std::unique_ptr<ShaderProgram> m_IsmRankingShaderProgram;

		static float s_ISMCoverageThreshold;
		static float s_ISMSplatWorldRadius;
		static float s_LowISMCoverageThreshold;
		static float s_LowISMSplatWorldRadius;

		UINT m_SMCount;
		UINT m_ISMCount;
	};
}