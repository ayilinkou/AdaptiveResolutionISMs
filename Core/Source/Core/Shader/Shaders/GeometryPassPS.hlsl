#include "Common.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D emissiveTexture : register(t2);

struct MaterialData
{
	float3 AlbedoColor;
	int bHasAlbedoTexture;
	float Specular;
	int bHasSpecularTexture;
	float Opacity;
	int bHasEmissiveTexture;
	float3 EmissiveColor;
	float Padding;
};

cbuffer Material : register(b1)
{
	MaterialData mat;
};

struct PS_In
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

struct PS_Out
{
	float4 Color : SV_TARGET0;
	float4 NormalSpec : SV_TARGET1;
	float4 Emissive : SV_TARGET2;
};

PS_Out main(PS_In p)
{
	PS_Out o;
	
	if (mat.bHasAlbedoTexture)
	{
		o.Color = albedoTexture.Sample(linearSampler, p.uv);
	}
	else
	{
		o.Color = float4(mat.AlbedoColor, mat.Opacity);
	}
	clip(o.Color.a < 0.1f ? -1.f : 1.f); // might be too harsh, play around with this number
	
	if (mat.bHasSpecularTexture)
	{
		o.NormalSpec.a = specularTexture.Sample(linearSampler, p.uv).r;
	}
	else
	{
		o.NormalSpec.a = mat.Specular;
	}
	
	if (mat.bHasEmissiveTexture)
	{
		o.Emissive = float4(emissiveTexture.Sample(linearSampler, p.uv).rgb, 1.f);
	}
	else
	{
		o.Emissive = float4(mat.EmissiveColor, 1.f);
	}
	
	o.NormalSpec.xyz = normalize(p.normal);
	return o;
}