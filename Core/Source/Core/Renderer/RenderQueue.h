#pragma once

#include <unordered_map>

#include "d3d11.h"
#include "wrl.h"
#include "DirectXMath.h"

#include "Core/Utility/Constants.h"

namespace Core
{
	class Model;
	class ModelData;
	
	class RenderQueue
	{
	private:
		struct ModelLocalBufferData
		{
			UINT ModelLocalCount;
			DirectX::XMFLOAT3 Padding;
			DirectX::XMMATRIX modelLocalTransforms[MAX_MODEL_LOCAL_COUNT];
		};

		enum RenderPassType
		{
			Main,
			Shadow
		};

	public:
		RenderQueue();

		void PopulateRenderQueue();
		void RenderMainPass();
		void RenderShadowPass();

	private:
		void Init();
		void CreateBuffers();

		void RenderPass(RenderPassType passType);

		void UpdateLocalCBuffer(const std::vector<DirectX::XMMATRIX>& modelLocalTransformsT);
		void UpdateWorldCBuffer(const std::vector<DirectX::XMMATRIX>& modelWorldTransformsT);
		
		void Add(Model* pModel);

	private:
		std::unordered_map<ModelData*, std::vector<DirectX::XMMATRIX>> m_ModelWorldTransformsMapT;

		// TODO: structured buffers would probably be better for both of these
		// per model mesh transform (model space)
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ModelLocalCBuffer;

		// per instance transform (world space)
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ModelWorldCBuffer;
	};
}