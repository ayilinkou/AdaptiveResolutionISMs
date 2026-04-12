#include "../../Utility/Constants.h"
#include "GlobalCBuffer.hlsl"
#include "Common.hlsl"

Texture2D normalSpecTexture : register(t0);
Texture2D depthTexture : register(t1);

RWStructuredBuffer<uint> lightScores : register(u0);

struct Reservoir
{
	uint LightID;
	float WeightSum;
	uint NumSamples;
};

uint WangHash(uint seed)
{
	seed = (seed ^ 61u) ^ (seed >> 16);
	seed *= 9u;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2du;
	seed = seed ^ (seed >> 15);
	return seed;
}

float RandomFloat(inout uint state)
{
	state = WangHash(state);
	return state / 4294967296.f;
}

float SampleISM(uint lightID, float3 worldPos)
{
	SpotLight spotLight = globalCBuffer.Lights.SpotLights[lightID];

	float4 lightClip = mul(float4(worldPos, 1.f), spotLight.ViewProj);
	float3 lightNDC = lightClip.xyz / lightClip.w;
	float currentDepth = lightNDC.z;
	float2 uv;
	uv.x = lightNDC.x * 0.5f + 0.5f;
	uv.y = -lightNDC.y * 0.5f + 0.5f;
	
	if (uv.x < 0.f || uv.x >= 1.f || uv.y < 0.f || uv.y >= 1.f)
		return 0.f; // outside of the shadow map, light cannot contribute to this pixel
	
	float bias = spotLight.MaxBias;
	float depth = spotShadowMaps.Load(int4(uv, lightID, 0)).r;
	return depth > currentDepth - bias ? 1.f : 0.f;
}

float ComputeLightWeight(uint lightID, float3 worldPos, float3 normal)
{
	SpotLight spotLight = globalCBuffer.Lights.SpotLights[lightID];

	float3 L = spotLight.Position - worldPos;
	float distSq = dot(L, L);
	float dist = sqrt(distSq);
	L /= dist;

	float geometryTerm = dot(normal, L);

	if (geometryTerm <= 0.f)
		return 0.f;

	float attenuation = 1.f / (distSq + 1e-4); // TODO: rough estimate, doesn't need to be exact, just fast, change if not close enough
	
	float visibility = SampleISM(lightID, worldPos);
	float intensity = spotLight.Intensity;

	float weight = intensity * geometryTerm * attenuation * visibility;

	return max(weight, 0.f);
}

[numthreads(ISM_RANKING_THREADS_X, ISM_RANKING_THREADS_Y, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
	uint2 texel = id.xy;
	
	uint2 screenSize = uint2(globalCBuffer.ScreenWidth, globalCBuffer.ScreenHeight);
	// this should never be true, but just in case
	if (texel.x >= screenSize.x || texel.y >= screenSize.y)
		return;
	
	float2 uv = float2((float)texel.x / (float)screenSize.x, (float)texel.y / (float)screenSize.y);
	float depth = depthTexture[texel].r;
	if (depth == 0.f)
		return;
	
	float4 hNDC = float4(uv.x * 2.f - 1.f, (1.f - (uv.y)) * 2.f - 1.f, depth, 1.f);
	float4 viewPos = mul(hNDC, globalCBuffer.Camera.InverseProj);
	viewPos /= viewPos.w;
	float3 worldPos = mul(viewPos, globalCBuffer.Camera.InverseView).xyz;
	
	float3 worldNormal = normalize(normalSpecTexture[texel].xyz);
	
	Reservoir reservoir;
	reservoir.LightID = 0u;
	reservoir.WeightSum = 0.f;
	reservoir.NumSamples = 0u;
	
	uint rngState = texel.x + texel.y * 4096 + asuint(globalCBuffer.Time);
	
	const uint numLights = globalCBuffer.Lights.SpotLightCount;
	for (uint i = 0; i < numLights; i++) // TODO: eventually this must loop over M candidate lights instead of all lights
	{
		// this doesn't have to be exact, just needs to be fast
		float weight = ComputeLightWeight(i, worldPos, worldNormal);

		if (weight <= 0.f)
			continue;

		reservoir.WeightSum += weight;
		reservoir.NumSamples++;
		
		float p = weight / reservoir.WeightSum;
		
		if (RandomFloat(rngState) < p)
			reservoir.LightID = i;
	}
	
	if (reservoir.WeightSum == 0.f)
		return;
	
	InterlockedAdd(lightScores[reservoir.LightID], 1);
}
