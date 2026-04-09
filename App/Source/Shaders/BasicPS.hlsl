#include "Common.hlsl"

Texture2D albedoTexture : register(t0);

struct PS_In
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

float4 main(PS_In p) : SV_TARGET
{		
	return float4(albedoTexture.Sample(linearSampler, p.uv).rgb, 1.f);
}