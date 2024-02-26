#include "MathsConstants.hlsli"
#include "Common.hlsli"

struct VertexIn
{
	float3 Position			: POSITION;
	float2 UV				: TEXCOORD;
	float3 Normal			: NORMAL;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float3 Normal			: NORMAL;
	float3x3 TBN			: TANGENT_BASIS;
	float2 UV				: TEXCOORD;
};

float4 GenerateTangent(float3 normal)
{
	float3 tangent = cross(normal, float3(0.0f, 1.0f, 0.0));
	tangent = normalize(lerp(cross(normal, float3(1.0f, 0.0f, 0.0f)), tangent, step(MIN_FLOAT_VALUE, dot(tangent, tangent))));

	return float4(tangent, 1.0f);
}

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;
	float4 worldPosition = mul(Model, float4(v.Position, 1.0f));
	output.Position = mul(CameraViewProjection, worldPosition);

	float3 tangent = GenerateTangent(v.Normal).xyz;
	float3 bitangent = cross(v.Normal, tangent);
	
	float3x3 mat = (float3x3)Model;
	float3 T = normalize(mul(tangent, mat));
	float3 N = normalize(mul(v.Normal, mat));
	float3 B = normalize(mul(bitangent, mat));
	output.Normal = N;
	output.TBN = transpose(float3x3(T, B, N));
	output.UV = v.UV;
	return output;
}

cbuffer Textures : register(b0, space1)
{
	uint AlbedoTexture;
	uint NormalTexture;
	uint MRATexture;
	uint EmissiveTexture;
}

cbuffer MaterialProperties : register(b1, space1)
{
	float4 AlbedoColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	float Roughness = 1.0f;
	float Metalness = 0.0f;
	float EmissiveIntensity = 1.0f;
	float AlphaCutoffThreshold = 0.01f;
}

struct PixelOut
{
	float4 Albedo				: SV_TARGET0;
	float4 Normal				: SV_TARGET1;
	float4 MetalicRoughnessAO	: SV_TARGET2;
	float4 Emission				: SV_TARGET3;
};

PixelOut PS_Main(VertexOut input)
{
	Texture2D albedoTexture = ResourceDescriptorHeap[AlbedoTexture];
	Texture2D normalTexture = ResourceDescriptorHeap[NormalTexture];
	Texture2D mraTexture = ResourceDescriptorHeap[MRATexture];
	Texture2D emissiveTexture = ResourceDescriptorHeap[EmissiveTexture];

	PixelOut pixel;

	float4 albedo = albedoTexture.Sample(Sampler, input.UV);
	float3 normal = NormalTexture ? normalize(mul(input.TBN, normalTexture.Sample(Sampler, input.UV).rgb * 2.0f - 1.0f)) : input.Normal;
	float4 mra = mraTexture.Sample(Sampler, input.UV);
	float3 emissive = emissiveTexture.Sample(Sampler, input.UV).rgb;

	pixel.Albedo = albedo * AlbedoColor;
	pixel.Normal = float4(normal, 1.0f);
	pixel.MetalicRoughnessAO = float4(mra.r * Metalness, mra.g * Roughness, mra.b, 1.0f);
	pixel.Emission = float4(emissive, EmissiveIntensity / 255.0f);
	return pixel;
}
