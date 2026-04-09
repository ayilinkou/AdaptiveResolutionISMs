#pragma once

#include "DirectXMath.h"

namespace Core {	
	class Mesh;
	class Material;
	
	struct RenderItem
	{
	public:

	private:
		Mesh* pMesh = nullptr;
		Material* pMat = nullptr;
		DirectX::XMMATRIX WorldAccumulated = DirectX::XMMatrixIdentity();
	};
}
