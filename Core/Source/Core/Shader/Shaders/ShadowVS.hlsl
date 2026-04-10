#include "Common.hlsl"

struct VS_In
{
	float3 pos : POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	
	uint instanceID : SV_InstanceID;
};

struct VS_Out
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION;
};

cbuffer ModelLocalBuffer : register(b1)
{
	uint modelLocalCount;
	float3 padding0;
	float4x4 localTransforms[MAX_MODEL_LOCAL_COUNT];
}

cbuffer ModelWorldBuffer : register(b2)
{
	float4x4 worldTransforms[MAX_MODEL_WORLD_COUNT];
}

cbuffer LightBuffer : register(b3)
{
	float4x4 lightViewProj;
	uint lightIndex;
	float3 padding1;
}

VS_Out main(VS_In v)
{	
	uint modelID = v.instanceID % modelLocalCount;
	uint instanceID = v.instanceID / modelLocalCount;
	
	VS_Out o;
	o.pos = mul(float4(v.pos, 1.f), localTransforms[modelID]); // to model space
	o.pos = mul(o.pos, worldTransforms[instanceID]); // to world space
	o.worldPos = o.pos.xyz;
	o.pos = mul(o.pos, lightViewProj);
	
	return o;
}