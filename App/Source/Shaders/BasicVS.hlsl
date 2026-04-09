#include "GlobalCBuffer.hlsl"

struct VS_In
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

struct VS_Out
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

VS_Out main(VS_In v)
{
	VS_Out o;
	o.pos = mul(float4(v.pos, 1.f), globalCBuffer.Camera.View);
	o.pos = mul(o.pos, globalCBuffer.Camera.Proj);
	
	o.normal = v.normal;
	o.uv = v.uv;
	
	return o;
}