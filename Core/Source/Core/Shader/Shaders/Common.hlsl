#include "GlobalCBuffer.hlsl"

SamplerState linearSampler : register(s0);
SamplerComparisonState shadowSampler : register(s1);

Texture2DArray dirShadowMaps : register(t3);
Texture2DArray spotShadowMaps : register(t4);
TextureCubeArray pointShadowMaps : register(t5);

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
	
	float bias = CalcBias(0.000005f, 0.0005f, worldNormal, -spotLight.Dir);
	float shadow = spotShadowMaps.SampleCmpLevelZero(shadowSampler, float3(shadowUV, lightIndex), currentDepth - bias);

	float geometryTerm = pow(dot(-spotLight.Dir, worldNormal) * 0.5f + 0.5f, 2.f); // Valve's half lambert
	float3 diffuse = spotLight.Color * pixelColor * geometryTerm;
	lightTotal += diffuse;

	float3 halfwayVec = normalize(pixelToCam - spotLight.Dir);
	float specularFactor = pow(saturate(dot(worldNormal, halfwayVec)), spotLight.SpecularPower);
	float3 specular = float4(spotLight.Color, 1.f) * specularFactor * reflectance;
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
	float shadow = pointShadowMaps.SampleCmpLevelZero(shadowSampler, float4(shadowMapDirNorm, lightIndex), normalizedDepth - bias);
	
	if (shadow == 0.f)
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
	float shadow = dirShadowMaps.SampleCmpLevelZero(shadowSampler, float3(shadowUV, lightIndex), currentDepth - bias);

	float geometryTerm = pow(dot(-dirLight.Dir, worldNormal) * 0.5f + 0.5f, 2.f); // Valve's half lambert
	float3 diffuse = dirLight.Color * pixelColor * geometryTerm;
	lightTotal += diffuse;

	float3 halfwayVec = normalize(pixelToCam - dirLight.Dir);
	float specularFactor = pow(saturate(dot(worldNormal, halfwayVec)), dirLight.SpecularPower);
	float3 specular = float4(dirLight.Color, 1.f) * specularFactor * reflectance;
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