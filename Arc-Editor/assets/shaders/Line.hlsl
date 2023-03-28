#include "Common.hlsli"

struct VertexIn
{
	float4 Position			: POSITION;
	float4 Color			: COLOR;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float4 Color			: COLOR;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	output.Position = mul(CameraViewProjection, float4(v.Position.xyz, 1.0));
	output.Color = v.Color;

	return output;
}

float4 PS_Main(VertexOut v) : SV_TARGET
{
	return v.Color;
}
