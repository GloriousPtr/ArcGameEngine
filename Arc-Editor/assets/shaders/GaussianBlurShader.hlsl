#include "Common.hlsli"

struct VertexIn
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD;
};

struct VertexOut
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;
	output.Position = v.Position;
	output.UV = v.UV;
	return output;
}

cbuffer Properties : register(b0, space1)
{
	uint MainTexture;
	uint Horizontal;
}

float4 PS_Main(VertexOut i) : SV_TARGET
{
	Texture2D mainTex = ResourceDescriptorHeap[MainTexture];
	
	float4 color = float4(1.0, 0.0, 1.0, 1.0);
	float width = 0.0f;
	float height = 0.0f;
	mainTex.GetDimensions(width, height);
	float2 texelSize = float2(1.0f, 1.0f) / float2(width, height);

	if (Horizontal)
	{
		float texelSizeX = texelSize.x * 2;
		// 9-tap gaussian blur on the downsampled source
		float4 c0 = mainTex.Sample(LinearPointClampSampler, i.UV - float2(texelSizeX * 4.0, 0.0));
		float4 c1 = mainTex.Sample(LinearPointClampSampler, i.UV - float2(texelSizeX * 3.0, 0.0));
		float4 c2 = mainTex.Sample(LinearPointClampSampler, i.UV - float2(texelSizeX * 2.0, 0.0));
		float4 c3 = mainTex.Sample(LinearPointClampSampler, i.UV - float2(texelSizeX * 1.0, 0.0));
		float4 c4 = mainTex.Sample(LinearPointClampSampler, i.UV);
		float4 c5 = mainTex.Sample(LinearPointClampSampler, i.UV + float2(texelSizeX * 1.0, 0.0));
		float4 c6 = mainTex.Sample(LinearPointClampSampler, i.UV + float2(texelSizeX * 2.0, 0.0));
		float4 c7 = mainTex.Sample(LinearPointClampSampler, i.UV + float2(texelSizeX * 3.0, 0.0));
		float4 c8 = mainTex.Sample(LinearPointClampSampler, i.UV + float2(texelSizeX * 4.0, 0.0));

		color = c0 * 0.01621622 + c1 * 0.05405405 + c2 * 0.12162162 + c3 * 0.19459459
					+ c4 * 0.22702703
					+ c5 * 0.19459459 + c6 * 0.12162162 + c7 * 0.05405405 + c8 * 0.01621622;
	}
	else
	{
		float texelSizeY = texelSize.y;
		// Optimized bilinear 5-tap gaussian on the same-sized source (9-tap equivalent)
		float4 c0 = mainTex.Sample(LinearPointClampSampler, i.UV - float2(0.0, texelSizeY * 3.23076923));
		float4 c1 = mainTex.Sample(LinearPointClampSampler, i.UV - float2(0.0, texelSizeY * 1.38461538));
		float4 c2 = mainTex.Sample(LinearPointClampSampler, i.UV);
		float4 c3 = mainTex.Sample(LinearPointClampSampler, i.UV + float2(0.0, texelSizeY * 1.38461538));
		float4 c4 = mainTex.Sample(LinearPointClampSampler, i.UV + float2(0.0, texelSizeY * 3.23076923));

		color = c0 * 0.07027027 + c1 * 0.31621622
                    + c2 * 0.22702703
                    + c3 * 0.31621622 + c4 * 0.07027027;
	}

	return float4(color.rgb, 1.0);
}
