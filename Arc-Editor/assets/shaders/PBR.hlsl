#include "Common.hlsli"

struct VertexIn
{
	float3 Position			: POSITION;
	float2 UV				: TEXCOORD;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Bitngent			: BITANGENT;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float2 UV				: TEXCOORD;
	float3 Normal			: NORMAL;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	const float4 worldPosition = mul(Model, float4(v.Position, 1.0));
	output.Position = mul(ViewProjection, worldPosition);
	output.UV = v.UV;
	output.Normal = v.Normal;

	return output;
}

cbuffer Textures : register(b0, space1)
{
	uint AlbedoTexture;
}

cbuffer MaterialProperties : register(b1, space1)
{
	float4 AlbedoColor;
	float AlphaCutoffThreshold;
}

float4 PS_Main(VertexOut input) : SV_TARGET
{
	const Texture2D<float4> albedoTexture = ResourceDescriptorHeap[AlbedoTexture];
	float4 albedo = albedoTexture.Sample(Sampler, input.UV);
	float4 color = albedo * AlbedoColor;
	color.a *= step(AlphaCutoffThreshold, albedo.a);

	return color;
}
