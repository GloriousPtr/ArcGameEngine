#include "ConstantData.hlsli"

struct VertexIn
{
	float4 Position			: POSITION;
	float4 Color			: COLOR;
	float4 UV				: TEXCOORD;	// xy: UV, zw: Tiling
	uint TexIndex			: TEXINDEX;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float4 Color			: COLOR;
	float4 UV				: TEXCOORD; // xy: UV, zw: Tiling
	uint TexIndex			: TEXINDEX;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	output.Position = mul(float4(v.Position.xyz, 1.0), ViewProjection);
	output.Color = v.Color;
	output.UV = v.UV;
	output.TexIndex = v.TexIndex;

	return output;
}

cbuffer Textures : register(b2, space0)
{
	uint AlbedoTextures[32];
}

float4 PS_Main(VertexOut v) : SV_TARGET
{
	Texture2D albedo = ResourceDescriptorHeap[NonUniformResourceIndex(AlbedoTextures[v.TexIndex])];
	float4 color = albedo.Sample(Sampler, v.UV.xy * v.UV.zw) * v.Color;
	return color;
}
