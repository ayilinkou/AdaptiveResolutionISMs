struct VS_In
{
	float3 Pos : POSITION;
	float2 UV : TEXCOORD0;
};

struct VS_Out
{
	float4 Pos : SV_POSITION;
	float2 UV : TEXCOORD0;
};

VS_Out main(VS_In v)
{
	VS_Out o;
	o.Pos = float4(v.Pos, 1.f);
	o.UV = v.UV;
	
	return o;
}