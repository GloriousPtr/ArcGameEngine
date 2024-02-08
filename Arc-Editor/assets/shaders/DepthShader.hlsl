#include "Common.hlsli"

struct VertexIn
{
	float3 Position			: POSITION;
	float2 UV				: TEXCOORD;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
};

cbuffer Properties : register(b0, space1)
{
	float4x4 ViewProjection;
}

float4 VS_Main(VertexIn i) : SV_POSITION
{
	float4 worldPosition = mul(Model, float4(i.Position, 1.0f));
	return mul(ViewProjection, worldPosition);
}
