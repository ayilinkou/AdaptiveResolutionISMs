#pragma once

#include <memory>

#include "d3d11.h"

#include "wrl.h"

#include "Core/Resource/Resource.h"

namespace Core {
	struct ShaderResource
	{
		std::unique_ptr<Resource> ShaderRes;
		Microsoft::WRL::ComPtr<ID3D10Blob> Bytecode;
	};
}
