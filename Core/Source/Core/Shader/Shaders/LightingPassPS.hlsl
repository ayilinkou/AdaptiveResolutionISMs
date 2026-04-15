#include "GlobalCBuffer.hlsl"
#include "Common.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D normalSpecTexture : register(t1);
Texture2D emissiveTexture : register(t2);
Texture2D depthTexture : register(t3);

struct PS_In
{
	float4 Pos : SV_POSITION;
	float2 UV : TEXCOORD0;
};

static const float GAMMA = 2.2f;

float4 main(PS_In p) : SV_TARGET
{
	float3 albedo = albedoTexture.Sample(pointSampler, p.UV).rgb;
	float4 normalSpec = normalSpecTexture.Sample(pointSampler, p.UV);
	float3 normalWS = normalSpec.xyz;
	float reflectance = normalSpec.a;
	float depth = depthTexture.Sample(pointSampler, p.UV).r;
	float3 emissive = emissiveTexture.Sample(pointSampler, p.UV).rgb;
    
	float4 hNDC = float4(p.UV.x * 2.f - 1.f, (1.f - (p.UV.y)) * 2.f - 1.f, depth, 1.f);
	float4 viewPos = mul(hNDC, globalCBuffer.Camera.InverseProj);
	viewPos /= viewPos.w;
	float3 worldPos = mul(viewPos, globalCBuffer.Camera.InverseView).xyz;
	float3 pixelToCam = normalize(globalCBuffer.Camera.Pos - worldPos);
    
	float3 light = CalcLight(albedo, worldPos, normalWS, pixelToCam, reflectance);
	
	float3 ambient = albedo * globalCBuffer.Lights.AmbientStrength * globalCBuffer.SkyColor;
	float3 color = light + ambient + emissive;
	
	return float4(pow(color, 1.f / GAMMA), 1.f);
}