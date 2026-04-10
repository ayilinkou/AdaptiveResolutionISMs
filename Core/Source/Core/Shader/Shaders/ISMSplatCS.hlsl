#include "../../Utility/Constants.h"

StructuredBuffer<float3> points : register(t0);

RWTexture2DArray<float2> ismTexture : register(u0); // r = depth, g = weight (used for pull push)

cbuffer SplatBuffer : register(b1)
{
	float4x4 lightView;
	float4x4 lightProj;
	uint pointCount;
	uint shadowRes;
	uint lightIndex;
	float padding;
};

static const float TEXEL_RADIUS = 0.02f;

[numthreads(ISM_SPLAT_THREADS_X, ISM_SPLAT_THREADS_Y, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
	uint2 texel = id.xy;
	if (texel.x >= shadowRes || texel.y >= shadowRes)
		return;
	
	float2 uv = (float2(texel) + 0.5f) / (float) shadowRes;
	float accumDepthWeight = ismTexture[uint3(texel, lightIndex)].r;
	float accumWeight = ismTexture[uint3(texel, lightIndex)].g;
	
	for (uint i = 0u; i < pointCount; i++)
	{
		float4 viewPos = mul(float4(points[i], 1.f), lightView);
		float depth = abs(viewPos.z);
		
		if (depth <= 0.f) // behind light view
			continue;
		
		float4 clipPos = mul(viewPos, lightProj);
		float2 ndc = clipPos.xy / clipPos.w; // bottom = -1, top = 1
		float2 pointUV = (float2(ndc.x, -ndc.y) * 0.5f + 0.5f); // bottom = 1, top = 0
		
		float2 deltaUV = pointUV - uv;
		float dist = length(deltaUV);
		
		float splatRadius = TEXEL_RADIUS / depth;

		if (dist < splatRadius)
		{
			float weight = exp(-(dist * dist) / (2.0f * splatRadius * splatRadius));
			accumDepthWeight += depth * weight;
			accumWeight += weight;
		}
	}
	
	ismTexture[uint3(texel, lightIndex)] = float2(accumDepthWeight, accumWeight);
	
	// for temporarily viewing the depth
	{
		float lightNear = 0.1f;
		float lightFar = 50.f;
		float depthNormalized = 1.f;
		float weight = 0.f;
		if (accumWeight > 1e-5f)
		{
			float depth = accumDepthWeight / accumWeight;
			depthNormalized = saturate((depth - lightNear) / (lightFar - lightNear));
			weight = 1.f;
		}

		ismTexture[uint3(texel, lightIndex)] = float2(depthNormalized, weight);
	}
}