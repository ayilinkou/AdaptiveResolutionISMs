#include "GlobalCBuffer.hlsl"

struct VS_In
{
	float3 pos : POSITION;
};

struct VS_Out
{
	float4 pos : SV_POSITION;
};

VS_Out main(VS_In v)
{
	VS_Out o;
	o.pos = mul(float4(v.pos, 1.f), GlobalCBuffer.Camera.View);
	o.pos = mul(o.pos, GlobalCBuffer.Camera.Proj);
	return o;
}