#ifndef GLOBALCBUFFER_HLSL
#define GLOBALCBUFFER_HLSL

#include "../../Utility/Constants.h"

struct CameraData
{
	float4x4 View;
	float4x4 Proj;
	float4x4 InverseView;
	float4x4 InverseProj;
	float3 Pos;
	float Padding;
};

struct PointLight
{
	float3 Color;
	float SpecularPower;
	float3 Position;
	float Intensity;
	float3 Attenuation;
	float Radius;
};

struct SpotLight
{
	float3 Color;
	float SpecularPower;
	float3 Position;
	float Intensity;
	float3 Attenuation;
	float Radius;
	float3 Dir;
	float CosInnerAngle;
	float CosOuterAngle;
	float NearZ;
	float MinBias;
	float MaxBias;
	float4x4 ViewProj;
	uint ShadowMapRes;
	uint GlobalID;
	float2 Padding;
};

struct DirectionalLight
{
	float3 Color;
	float SpecularPower;
	float3 Dir;
	float Intensity;
	float4x4 ViewProj;
	uint ShadowMapRes;
	float3 Padding;
};

struct LightData
{
	uint PointLightCount;
	uint SpotLightCount;
	uint DirectionalLightCount;
	float AmbientStrength;
	PointLight PointLights[MAX_POINT_LIGHT_COUNT];
	SpotLight SpotLights[MAX_SPOT_LIGHT_COUNT];
	DirectionalLight DirectionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
};

struct GlobalCBufferData
{
	CameraData Camera;
	LightData Lights;
	uint ScreenWidth;
	uint ScreenHeight;
	float NearZ;
	float FarZ;
	float Time;
	float3 Padding;
};

cbuffer GlobalCBuffer : register(b0)
{
	GlobalCBufferData globalCBuffer;
};

#endif