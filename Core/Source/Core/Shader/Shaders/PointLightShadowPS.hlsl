#include "GlobalCBuffer.hlsl"

struct PS_In
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION;
};

cbuffer LightBuffer : register(b1)
{
	float4x4 lightViewProj;
	uint lightIndex;
	float3 padding;
}

float main(PS_In p) : SV_TARGET
{
	float dist = length(p.worldPos - globalCBuffer.Lights.PointLights[lightIndex].Position);
	return dist / globalCBuffer.Lights.PointLights[lightIndex].Radius;
}