#include "GlobalCBuffer.hlsl"

struct PS_In
{
	float4 pos :SV_POSITION;
};

float4 main(PS_In p) : SV_TARGET
{		
	return float4(1.f, 0.f, abs(sin(GlobalCBuffer.Time)), 1.f);
}