#include "Common.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D specularTexture : register(t1);

struct MaterialData
{
	float3 AlbedoColor;
	int bHasAlbedoTexture;
	float Specular;
	int bHasSpecularTexture;
	float Opacity;
	float Padding;
};

cbuffer Material : register(b1)
{
	MaterialData mat;
};

struct PS_In
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

float4 main(PS_In p) : SV_TARGET
{		
	float4 color;
	if (mat.bHasAlbedoTexture)
	{
		color = albedoTexture.Sample(linearSampler, p.uv);
	}
	else
	{
		color = float4(mat.AlbedoColor, mat.Opacity);
	}
	clip(color.a < 0.1f ? -1.f : 1.f); // might be too harsh, play around with this number
	
	return float4(color);
}