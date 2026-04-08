struct CameraData
{
	float4x4 View;
	float4x4 Proj;
	float3 CameraPos;
	float Padding;
};

struct GlobalCBufferData
{
	CameraData Camera;
	uint ScreenWidth;
	uint ScreenHeight;
	float NearZ;
	float FarZ;
	float Time;
	float3 Padding;
};

cbuffer GlobalCBuffer : register(b0)
{
	GlobalCBufferData GlobalCBuffer;
};