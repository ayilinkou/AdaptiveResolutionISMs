#include "../../Utility/Constants.h"
#include "Common.hlsl"

StructuredBuffer<float3> points : register(t0);

RWTexture2D<uint> ismDepth : register(u0);

cbuffer SplatBuffer : register(b1)
{
	float4x4 lightView;
	float4x4 lightProj;
	uint pointCount;
	uint shadowRes;
	uint lightIndex;
	float nearPlane;
	float farPlane;
	float splatRadiusWorld;
	float2 padding;
};

[numthreads(ISM_SPLAT_THREADS_X, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
	uint pointIndex = id.x;
	if (pointIndex >= pointCount)
		return;
	
	float3 pos = points[pointIndex];
	
	float4 viewPos = mul(float4(pos, 1.f), lightView);
	float depth = viewPos.z;
				
	float normalizedDepth = saturate((depth - nearPlane) / (farPlane - nearPlane));
	uint depthInt = (uint) (normalizedDepth * 0xFFFFFFFF); // mapped as uint for InterlockedMin()

	float4 clipPos = mul(viewPos, lightProj);
	float3 ndc = clipPos.xyz / clipPos.w; // bottom = -1, top = 1
	
	float projScale = lightProj._11;
	float texelRadius = splatRadiusWorld * projScale / max(depth, 0.0001f) * (float)shadowRes * 0.5f;
	int radius = (int)clamp(texelRadius, 1.f, 20.f); // TODO: maybe try without clamping?
	
	// frustum culling
	// We want to be conservative and still include points which might originate outside the frustum but might still be visible due to splat radius.
	const float bias = (2.f * (float)radius) / (float)shadowRes;
	if (abs(ndc.x) > 1.f + bias || abs(ndc.y) > 1.f + bias || ndc.z < 0.f || ndc.z > 1.f)
		return;
	
	float2 pointUV = (float2(ndc.x, -ndc.y) * 0.5f + 0.5f); // bottom = 1, top = 0
	float2 shadowUV = pointUV * (float)shadowRes;
	uint2 intShadowUV = (int2)(shadowUV);

	for (int y = -radius; y <= radius; y++)
	{
		for (int x = -radius; x <= radius; x++)
		{
			int2 texel = intShadowUV + int2(x, y);
			if (texel.x < 0 || texel.y < 0 || texel.x >= (int)shadowRes || texel.y >= (int)shadowRes)
				continue;
			
			if (x * x + y * y > radius * radius)
				continue;
			
			InterlockedMin(ismDepth[texel], depthInt);
		}
	}
}