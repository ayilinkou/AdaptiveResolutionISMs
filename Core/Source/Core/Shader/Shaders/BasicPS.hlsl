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
	float3 worldPos : POSITION;
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
	
	float specular;
	if (mat.bHasSpecularTexture)
	{
		specular = specularTexture.Sample(linearSampler, p.uv).r;
	}
	else
	{
		specular = mat.Specular;
	}
	
	float3 normalWS = normalize(p.normal);
	float3 pixelToCam = normalize(globalCBuffer.Camera.Pos - p.worldPos);
		
	float3 totalLight = float3(0.f, 0.f, 0.f);
	totalLight += CalcSpotLights(color.rgb, p.worldPos, normalWS, pixelToCam, specular);
	totalLight += CalcPointLights(color.rgb, p.worldPos, normalWS, pixelToCam, specular);
	totalLight += CalcDirectionalLights(color.rgb, p.worldPos, normalWS, pixelToCam, specular);
	
	float3 ambient = color.rgb * globalCBuffer.Lights.AmbientStrength;
	return float4(totalLight + ambient, 1.f);
}