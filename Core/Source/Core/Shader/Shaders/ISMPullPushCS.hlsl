#include "../../Utility/Constants.h"
#include "Common.hlsl"

Texture2DArray<float4> ismSrcMip : register(t0);

RWTexture2DArray<float4> ismDestMip : register(u0);

cbuffer PullPushBuffer : register(b1)
{
	uint lightIndex;
	uint destMipRes;
	uint shadowRes;
	float coverageThreshold;
};

[numthreads(ISM_PULLPUSH_THREADS_X, ISM_PULLPUSH_THREADS_Y, 1)]
void Pull(uint3 id : SV_DispatchThreadID)
{
	uint2 texel = id.xy;
	if (texel.x >= destMipRes || texel.y >= destMipRes)
		return;
	
	uint2 srcBaseUv = texel * 2u;
	
	float4 t0 = ismSrcMip.Load(int4(srcBaseUv + uint2(0u, 0u), lightIndex, 0));	// top left
	float4 t1 = ismSrcMip.Load(int4(srcBaseUv + uint2(1u, 0u), lightIndex, 0));	// top right
	float4 t2 = ismSrcMip.Load(int4(srcBaseUv + uint2(0u, 1u), lightIndex, 0));	// bottom left
	float4 t3 = ismSrcMip.Load(int4(srcBaseUv + uint2(1u, 1u), lightIndex, 0));	// bottom right
	
	SplatPixel pixel;
	pixel.minDepth = 1.f;
	pixel.maxDepth = 1.f;
	pixel.bValid = ISM_PIXEL_INVALID;
	pixel.coverage = 0.f;
	
	SplatPixel children[4] = {
		{ t0.r, t0.g, t0.b, t0.a },
		{ t1.r, t1.g, t1.b, t1.a },
		{ t2.r, t2.g, t2.b, t2.a },
		{ t3.r, t3.g, t3.b, t3.a }
	};
	
	float validCount = children[0].bValid + children[1].bValid + children[2].bValid + children[3].bValid;
	if (validCount == 0.f)
	{
		ismDestMip[uint3(texel, lightIndex)] = float4(pixel.minDepth, pixel.maxDepth, ISM_PIXEL_INVALID, pixel.coverage);
		return;
	}
	
	// find frontmost child
	float frontMin = FLT_MAX;
	for (int i = 0; i < 4; i++)
	{
		if (children[i].bValid && children[i].minDepth < frontMin)
			frontMin = children[i].minDepth;
	}
	
	// average only children whose depth interval intersects with the frontmost interval
	float sumMin = 0.f;
	float sumMax = 0.f;
	float sumCoverage = 0.f;
	float count = 0.f;
	
	for (int j = 0; j < 4; j++)
	{
		if (!children[j].bValid)
			continue;
		
		bool overlaps = children[j].maxDepth >= frontMin;
		if (overlaps)
		{
			sumMin += children[j].minDepth;
			sumMax += children[j].maxDepth;
			sumCoverage += children[j].coverage;
			count += 1.f;
		}
	}

	pixel.minDepth = sumMin / count;
	pixel.maxDepth = sumMax / count;
	pixel.bValid = ISM_PIXEL_VALID;
	pixel.coverage = sumCoverage / 4.f;
	
	ismDestMip[uint3(texel, lightIndex)] = float4(pixel.minDepth, pixel.maxDepth, pixel.bValid, pixel.coverage);
}

[numthreads(ISM_PULLPUSH_THREADS_X, ISM_PULLPUSH_THREADS_Y, 1)]
void Push(uint3 id : SV_DispatchThreadID)
{
	uint2 texel = id.xy;
	if (texel.x >= destMipRes || texel.y >= destMipRes)
		return;
	
	float4 dst = ismDestMip.Load(int4(texel, lightIndex, 0));
	SplatPixel dstPixel;
	dstPixel.minDepth = dst.r;
	dstPixel.maxDepth = dst.g;
	dstPixel.bValid = dst.b;
	dstPixel.coverage = dst.a;
	
	if (dstPixel.bValid)
	{
		if (destMipRes == shadowRes)
		{
			 // last push pass, convert depth from normalized view Z to ndc Z since lighting calculation expects non-linear Z
			float near = globalCBuffer.Lights.SpotLights[lightIndex].NearZ;
			float far = globalCBuffer.Lights.SpotLights[lightIndex].Radius;
			float viewZ = near + dstPixel.minDepth * (far - near);
			float ndcZ = saturate(far / (far - near) - (near * far) / ((far - near) * viewZ));
			ismDestMip[uint3(texel, lightIndex)] = float4(ndcZ, dstPixel.maxDepth, ISM_PIXEL_VALID, dstPixel.coverage);
		}
		return;
	}
	
	uint2 srcUv = texel / 2u;
	float4 src = ismSrcMip.Load(int4(srcUv, lightIndex, 0));
	SplatPixel srcPixel;
	srcPixel.minDepth = src.r;
	srcPixel.maxDepth = src.g;
	srcPixel.bValid = src.b;
	srcPixel.coverage = src.a;
	
	if (srcPixel.bValid && srcPixel.coverage > coverageThreshold)
	{
		if (destMipRes == shadowRes)
		{
			 // last push pass, convert depth from normalized view Z to ndc Z since lighting calculation expects non-linear Z
			float near = globalCBuffer.Lights.SpotLights[lightIndex].NearZ;
			float far = globalCBuffer.Lights.SpotLights[lightIndex].Radius;
			float viewZ = near + dstPixel.minDepth * (far - near);
			float ndcZ = saturate(far / (far - near) - (near * far) / ((far - near) * viewZ));
			ismDestMip[uint3(texel, lightIndex)] = float4(ndcZ, dstPixel.maxDepth, ISM_PIXEL_VALID, dstPixel.coverage);
		}
		ismDestMip[uint3(texel, lightIndex)] = float4(srcPixel.minDepth, srcPixel.maxDepth, ISM_PIXEL_VALID, srcPixel.coverage);
	}	
}