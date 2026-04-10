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
	
	enum RenderPassType
	{
		Main,
		Shadow
	};

	enum class ShadowType
	{
		ShadowMap,
		ISM,
		None
	};

	class RenderQueue
	{
	private:
		struct ModelLocalBufferData
		{
			UINT ModelLocalCount;
			DirectX::XMFLOAT3 Padding;
			DirectX::XMMATRIX modelLocalTransforms[MAX_MODEL_LOCAL_COUNT];
		};

		struct SplatBufferData
		{
			DirectX::XMMATRIX LightView;
			DirectX::XMMATRIX LightProj;
			UINT PointCount;
			UINT ShadowRes;
			UINT LightIndex;
			float Padding;
		};

	public:
		RenderQueue();

		void PopulateRenderQueue();
		void RenderMainPass(ShadowType shadowType);
		void RenderShadowPass(ShadowType shadowType);

	private:
		void Init();
		void CreateBuffers();

		void RenderPass(RenderPassType passType, ShadowType shadowType = ShadowType::None);

		void UpdateLocalCBuffer(const std::vector<DirectX::XMMATRIX>& modelLocalTransformsT);
		void UpdateWorldCBuffer(const std::vector<DirectX::XMMATRIX>& modelWorldTransformsT);
		void UpdateSplatCBuffer(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, const UINT pointCount,
			const UINT shadowRes, const UINT lightIndex);
		
		void Add(Model* pModel);

		void ShadowMapPass();
		void ISMPass();
		void DispatchISMSplat(const DirectX::XMMATRIX& viewT, const DirectX::XMMATRIX& projT, const UINT ismRes, const UINT lightIndex);

	private:
		std::unordered_map<ModelData*, std::vector<DirectX::XMMATRIX>> m_ModelWorldTransformsMapT;

		// TODO: structured buffers would probably be better for both of these
		// per model mesh transform (model space)
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ModelLocalCBuffer;

		// per instance transform (world space)
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ModelWorldCBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ISMSplatCBuffer;

		std::unique_ptr<ShaderProgram> m_IsmSplatShaderProgram;
	};
}