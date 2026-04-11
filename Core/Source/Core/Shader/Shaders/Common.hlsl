#ifndef COMMON_HLSL
#define COMMON_HLSL

#include "GlobalCBuffer.hlsl"

#define FLT_MAX 3.402823466e+38

static const float ISM_PIXEL_INVALID = 0.f;
static const float ISM_PIXEL_VALID = 1.f;

SamplerState pointSampler : register(s0);
SamplerState linearSampler : register(s1);
SamplerState shadowSampler : register(s2);

Texture2DArray dirShadowMaps : register(t4);
Texture2DArray spotShadowMaps : register(t5);
TextureCubeArray pointShadowMaps : register(t6);

struct SplatPixel
{
	float minDepth;
	float maxDepth;
	float bValid;
	float coverage;
};

float pcf(int radius, uint shadowMapRes, float2 shadowUV, float bias, float currentDepth, uint lightIndex, Texture2DArray shadowMaps)
{
	// radius of 1 gives a 3x3 kernel
	float2 texelSize = float2(1.f / (float)shadowMapRes, 1.f / (float)shadowMapRes);
	float shadow = 0.f;
	float sampleCount = 0.f;

	[loop]
	for (int y = -radius; y <= radius; ++y)
	{
		[loop]
		for (int x = -radius; x <= radius; ++x)
		{
			float2 offset = float2(x, y) * texelSize;
			float2 uv = shadowUV + offset;

			float depth = shadowMaps.Sample(shadowSampler, float3(uv, lightIndex)).r;

			shadow += depth > currentDepth - bias ? 1.f : 0.f;
			sampleCount += 1.f;
		}
	}

	return shadow / sampleCount;
}

float CalcBias(float minBias, float slopeBias, float3 worldNormal, float3 pixelToLight)
{
	float NdotL = saturate(dot(worldNormal, pixelToLight));
	return max(minBias, slopeBias * (1.f - NdotL));
}

float3 _CalcSpotLight(uint lightIndex, float3 pixelColor, float3 worldPos, float3 worldNormal, float3 pixelToCam, float reflectance)
{
	const SpotLight spotLight = globalCBuffer.Lights.SpotLights[lightIndex];
	float3 lightTotal = float3(0.f, 0.f, 0.f);
	
	float d = distance(worldPos, spotLight.Position);
	if (d > spotLight.Radius)
		return float3(0.f, 0.f, 0.f);
	
	float distAttenuation = 1.f / (spotLight.Attenuation.x * d * d + spotLight.Attenuation.y * d + spotLight.Attenuation.z);
	
	float3 lightToPixel = normalize(worldPos - spotLight.Position);
	float cosToPixel = dot(lightToPixel, spotLight.Dir);
	float denom = max(spotLight.CosInnerAngle - spotLight.CosOuterAngle, 1e-5f); // guards against dividing by zero
	float coneAttenuation = saturate((cosToPixel - spotLight.CosOuterAngle) / denom);
	if (coneAttenuation <= 0.f)
		return float3(0.f, 0.f, 0.f);
	
	float4 lightClip = mul(float4(worldPos, 1.f), spotLight.ViewProj);
	float3 lightNDC = lightClip.xyz / lightClip.w;
	float currentDepth = lightNDC.z;
	float2 shadowUV;
	shadowUV.x = lightNDC.x * 0.5f + 0.5f;
	shadowUV.y = -lightNDC.y * 0.5f + 0.5f;
	
	float bias = CalcBias(spotLight.MinBias, spotLight.MaxBias, worldNormal, -spotLight.Dir);
	float shadow = pcf(1, spotLight.ShadowMapRes, shadowUV, bias, currentDepth, lightIndex, spotShadowMaps);

	if (shadow <= 0.f)
		return float3(0.f, 0.f, 0.f);

	float geometryTerm = pow(dot(-spotLight.Dir, worldNormal) * 0.5f + 0.5f, 2.f); // Valve's half lambert
	float3 diffuse = spotLight.Color * pixelColor * geometryTerm;
	lightTotal += diffuse;

	float3 halfwayVec = normalize(pixelToCam - spotLight.Dir);
	float specularFactor = pow(saturate(dot(worldNormal, halfwayVec)), spotLight.SpecularPower);
	float3 specular = spotLight.Color * specularFactor * reflectance;
	lightTotal += specular;
	
	return lightTotal * spotLight.Intensity * distAttenuation * coneAttenuation * shadow;
}

float3 CalcSpotLights(float3 pixelColor, float3 worldPos, float3 worldNormal, float3 pixelToCam, float reflectance)
{
	float3 totalLight = float3(0.f, 0.f, 0.f);
	for (uint i = 0u; i < globalCBuffer.Lights.SpotLightCount; i++)
	{
		totalLight += _CalcSpotLight(i, pixelColor, worldPos, worldNormal, pixelToCam, reflectance);
	}
	return totalLight;
}

float3 _CalcPointLight(uint lightIndex, float3 pixelColor, float3 worldPos, float3 worldNormal, float3 pixelToCam, float reflectance)
{
	PointLight pLight = globalCBuffer.Lights.PointLights[lightIndex];
	float3 lightTotal = float3(0.f, 0.f, 0.f);
	
	float d = distance(worldPos, pLight.Position);
	if (d > pLight.Radius)
		return float3(0.f, 0.f, 0.f);
	
	float3 shadowMapDir = worldPos - pLight.Position;
	float shadowMapDist = length(shadowMapDir);
	float3 shadowMapDirNorm = normalize(shadowMapDir);
	float normalizedDepth = shadowMapDist / pLight.Radius;
	
	float bias = CalcBias(0.0005f, 0.005f, worldNormal, -shadowMapDirNorm);
	float depth = pointShadowMaps.Sample(shadowSampler, float4(shadowMapDirNorm, lightIndex)).r;
	
	float shadow = depth > normalizedDepth - bias ? 1.f : 0.f;
	if (shadow <= 0.f)
		return float3(0.f, 0.f, 0.f);
	
	float attenuation = 1.f / (pLight.Attenuation.x * d * d + pLight.Attenuation.y * d + pLight.Attenuation.z);
	
	float3 pixelToLight = normalize(pLight.Position - worldPos);
	float geometryTerm = pow(dot(pixelToLight, worldNormal) * 0.5 + 0.5, 2.f); // Valve's half lambert
	float3 diffuse = pLight.Color * pixelColor * geometryTerm;
	lightTotal += diffuse;
		
	float3 halfwayVec = normalize(pixelToCam + pixelToLight);
	float specularFactor = pow(saturate(dot(worldNormal, halfwayVec)), pLight.SpecularPower);
	float3 specular = pLight.Color * specularFactor * reflectance;
	lightTotal += specular;
	
	return lightTotal * pLight.Intensity * attenuation * shadow;
}

float3 CalcPointLights(float3 pixelColor, float3 worldPos, float3 worldNormal, float3 pixelToCam, float reflectance)
{
	float3 totalLight = float3(0.f, 0.f, 0.f);
	for (uint i = 0u; i < globalCBuffer.Lights.PointLightCount; i++)
	{
		totalLight += _CalcPointLight(i, pixelColor, worldPos, worldNormal, pixelToCam, reflectance);
	}
	return totalLight;
}

float3 _CalcDirectionalLight(uint lightIndex, float3 pixelColor, float3 worldPos, float3 worldNormal, float3 pixelToCam, float reflectance)
{
	const DirectionalLight dirLight = globalCBuffer.Lights.DirectionalLights[lightIndex];
	float3 lightTotal = float3(0.f, 0.f, 0.f);
	
	float4 lightClip = mul(float4(worldPos, 1.f), dirLight.ViewProj);
	float3 lightNDC = lightClip.xyz / lightClip.w; // this might not be needed since using orthographic proj, so w should be 1
	float currentDepth = lightNDC.z;
	float2 shadowUV;
	shadowUV.x = lightNDC.x * 0.5f + 0.5f;
	shadowUV.y = -lightNDC.y * 0.5f + 0.5f;
	
	float bias = CalcBias(0.0005f, 0.0001f, worldNormal, -dirLight.Dir);
	float shadow = pcf(1, dirLight.ShadowMapRes, shadowUV, bias, currentDepth, lightIndex, dirShadowMaps);
	
	if (shadow <= 0.f)
		return float3(0.f, 0.f, 0.f);

	float geometryTerm = pow(dot(-dirLight.Dir, worldNormal) * 0.5f + 0.5f, 2.f); // Valve's half lambert
	float3 diffuse = dirLight.Color * pixelColor * geometryTerm;
	lightTotal += diffuse;

	float3 halfwayVec = normalize(pixelToCam - dirLight.Dir);
	float specularFactor = pow(saturate(dot(worldNormal, halfwayVec)), dirLight.SpecularPower);
	float3 specular = dirLight.Color * specularFactor * reflectance;
	lightTotal += specular;
	
	return lightTotal * dirLight.Intensity * shadow;
}

float3 CalcDirectionalLights(float3 pixelColor, float3 worldPos, float3 worldNormal, float3 pixelToCam, float reflectance)
{
	float3 totalLight = float3(0.f, 0.f, 0.f);
	for (uint i = 0u; i < globalCBuffer.Lights.DirectionalLightCount; i++)
	{
		totalLight += _CalcDirectionalLight(i, pixelColor, worldPos, worldNormal, pixelToCam, reflectance);
	}
	return totalLight;
}

float3 CalcLight(float3 pixelColor, float3 worldPos, float3 worldNormal, float3 pixelToCam, float reflectance)
{
	float3 totalLight = float3(0.f, 0.f, 0.f);
	totalLight += CalcSpotLights(pixelColor, worldPos, worldNormal, pixelToCam, reflectance);
	totalLight += CalcPointLights(pixelColor, worldPos, worldNormal, pixelToCam, reflectance);
	totalLight += CalcDirectionalLights(pixelColor, worldPos, worldNormal, pixelToCam, reflectance);
	return totalLight;
}

#endif