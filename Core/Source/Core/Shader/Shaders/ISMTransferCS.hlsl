#include "../../Utility/Constants.h"
#include "Common.hlsl"

Texture2D<uint> ismDepth : register(t0);

RWTexture2DArray<float4> ismMipZero : register(u0);

cbuffer SplatBuffer : register(b1)
{
	float4x4 lightView;
	float4x4 lightProj;
	uint pointCount;
	uint shadowRes;
	uint lightIndex;
	float nearPlane;
	float farPlane;
	float padding;
};

[numthreads(ISM_PULLPUSH_THREADS_X, ISM_PULLPUSH_THREADS_Y, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
	uint2 texel = id.xy;
	if (texel.x >= shadowRes || texel.y >= shadowRes)
		return;
	
	SplatPixel pixel;
	pixel.minDepth = 1.f;
	pixel.maxDepth = 1.f;
	pixel.bValid = ISM_PIXEL_INVALID;
	pixel.coverage = 0.f;
	
	uint depthInt = ismDepth[texel];
	if (depthInt < 0xFFFFFFFF)
	{
		// valid value, point landed here
		float normalizedDepth = depthInt / 4294967295.f; // 0xFFFFFFFF as float
		
		// normalized depth to view space z
		float viewZ = nearPlane + normalizedDepth * (farPlane - nearPlane);
		
		// min and max depth in view space
		float minZ = max(nearPlane, viewZ - ISM_SPLAT_WORLD_RADIUS);
		float maxZ = min(farPlane, viewZ + ISM_SPLAT_WORLD_RADIUS);
		
		// normalize
		float minDepthNormalized = saturate((minZ - nearPlane) / (farPlane - nearPlane));
		float maxDepthNormalized = saturate((maxZ - nearPlane) / (farPlane - nearPlane));
		
		// store
		pixel.minDepth = minDepthNormalized;
		pixel.maxDepth = maxDepthNormalized;
		pixel.bValid = ISM_PIXEL_VALID;
		pixel.coverage = 1.f;
	}	
	
	ismMipZero[uint3(texel, lightIndex)] = float4(pixel.minDepth, pixel.maxDepth, pixel.bValid, pixel.coverage);
}