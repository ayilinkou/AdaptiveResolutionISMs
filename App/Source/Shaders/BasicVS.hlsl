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
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

cbuffer ModelLocalBuffer : register(b1)
{
	uint modelLocalCount;
	float3 padding;
	float4x4 localTransforms[MAX_MODEL_LOCAL_COUNT];
}

cbuffer ModelWorldBuffer : register(b2)
{
	float4x4 worldTransforms[MAX_MODEL_WORLD_COUNT];
}

VS_Out main(VS_In v)
{
	VS_Out o;
	
	uint modelID = v.instanceID % modelLocalCount;
	uint instanceID = v.instanceID / modelLocalCount;
	
	o.pos = mul(float4(v.pos, 1.f), localTransforms[modelID]); // to model space
	o.pos = mul(o.pos, worldTransforms[instanceID]); // to world space
	o.pos = mul(o.pos, globalCBuffer.Camera.View);
	o.pos = mul(o.pos, globalCBuffer.Camera.Proj);
	
	o.normal = mul(float4(v.normal, 0.f), localTransforms[modelID]).xyz;
	o.normal = mul(float4(o.normal, 0.f), worldTransforms[instanceID]).xyz;
	o.uv = v.uv;
	
	return o;
}