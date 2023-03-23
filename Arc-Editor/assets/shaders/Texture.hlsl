#include "Common.hlsli"

struct VertexIn
{
	float4 Position			: POSITION;
	float4 Color			: COLOR;
	float2 UV				: TEXCOORD;
	float4 TilingOffset		: TILINGOFFSET;
	uint TexIndex			: TEXINDEX;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float4 Color			: COLOR;
	float2 UV				: TEXCOORD;
	float4 TilingOffset		: TILINGOFFSET;
	uint TexIndex			: TEXINDEX;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	output.Position = mul(float4(v.Position.xyz, 1.0), ViewProjection);
	output.Color = v.Color;
	output.UV = v.UV;
	output.TilingOffset = v.TilingOffset;
	output.TexIndex = v.TexIndex;

	return output;
}

cbuffer Textures : register(b0, space1)
{
	uint AlbedoTexture0;
	uint AlbedoTexture1;
	uint AlbedoTexture2;
	uint AlbedoTexture3;
	uint AlbedoTexture4;
	uint AlbedoTexture5;
	uint AlbedoTexture6;
	uint AlbedoTexture7;
	uint AlbedoTexture8;
	uint AlbedoTexture9;
	uint AlbedoTexture10;
	uint AlbedoTexture11;
	uint AlbedoTexture12;
	uint AlbedoTexture13;
	uint AlbedoTexture14;
	uint AlbedoTexture15;
	uint AlbedoTexture16;
	uint AlbedoTexture17;
	uint AlbedoTexture18;
	uint AlbedoTexture19;
	uint AlbedoTexture20;
	uint AlbedoTexture21;
	uint AlbedoTexture22;
	uint AlbedoTexture23;
	uint AlbedoTexture24;
	uint AlbedoTexture25;
	uint AlbedoTexture26;
	uint AlbedoTexture27;
	uint AlbedoTexture28;
	uint AlbedoTexture29;
	uint AlbedoTexture30;
	uint AlbedoTexture31;
}

float4 PS_Main(VertexOut v) : SV_TARGET
{
	uint texIndex = 0;
	switch (v.TexIndex)
	{
		case 0: texIndex = AlbedoTexture0; break;
		case 1: texIndex = AlbedoTexture1; break;
		case 2: texIndex = AlbedoTexture2; break;
		case 3: texIndex = AlbedoTexture3; break;
		case 4: texIndex = AlbedoTexture4; break;
		case 5: texIndex = AlbedoTexture5; break;
		case 6: texIndex = AlbedoTexture6; break;
		case 7: texIndex = AlbedoTexture7; break;
		case 8: texIndex = AlbedoTexture8; break;
		case 9: texIndex = AlbedoTexture9; break;
		case 10: texIndex = AlbedoTexture10; break;
		case 11: texIndex = AlbedoTexture11; break;
		case 12: texIndex = AlbedoTexture12; break;
		case 13: texIndex = AlbedoTexture13; break;
		case 14: texIndex = AlbedoTexture14; break;
		case 15: texIndex = AlbedoTexture15; break;
		case 16: texIndex = AlbedoTexture16; break;
		case 17: texIndex = AlbedoTexture17; break;
		case 18: texIndex = AlbedoTexture18; break;
		case 19: texIndex = AlbedoTexture19; break;
		case 20: texIndex = AlbedoTexture20; break;
		case 21: texIndex = AlbedoTexture21; break;
		case 22: texIndex = AlbedoTexture22; break;
		case 23: texIndex = AlbedoTexture23; break;
		case 24: texIndex = AlbedoTexture24; break;
		case 25: texIndex = AlbedoTexture25; break;
		case 26: texIndex = AlbedoTexture26; break;
		case 27: texIndex = AlbedoTexture27; break;
		case 28: texIndex = AlbedoTexture28; break;
		case 29: texIndex = AlbedoTexture29; break;
		case 30: texIndex = AlbedoTexture30; break;
		case 31: texIndex = AlbedoTexture31; break;
		default: return float4(1.0, 0.0, 1.0, 1.0);
	}

	Texture2D albedo = ResourceDescriptorHeap[NonUniformResourceIndex(texIndex)];
	float4 color = albedo.Sample(Sampler, v.UV.xy * v.TilingOffset.xy + v.TilingOffset.zw) * v.Color;
	return color;
}
