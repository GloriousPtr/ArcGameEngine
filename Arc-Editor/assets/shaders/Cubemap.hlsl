#include "Common.hlsli"
#include "MathsConstants.hlsli"

struct VertexIn
{
	float3 Position			: POSITION;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float3 VertexPosition	: POSITION;
};

cbuffer SkyboxData : register(b0, space1)
{
	float4x4 SkyboxViewProjection;
	float2 RotationIntensity;
}

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;
	output.Position = mul(SkyboxViewProjection, float4(v.Position, 1.0));
	output.VertexPosition = v.Position;

	return output;
}

TextureCube EnvironmentTexture : register(t0, space1);

float3 RotateVectorAboutY(float angle, float3 vec)
{
	angle = radians(angle);
	float3x3 rotationMatrix = { float3(cos(angle),0.0,sin(angle)),
							float3(0.0,1.0,0.0),
							float3(-sin(angle),0.0,cos(angle)) };
	return mul(rotationMatrix, vec);
}

float4 PS_Main(VertexOut input) : SV_TARGET
{
	float3 envVector = normalize(input.VertexPosition);
	float3 color = EnvironmentTexture.Sample(Sampler, RotateVectorAboutY(RotationIntensity.x, envVector)).rgb * RotationIntensity.y;
	color = pow(color, 1.0 / Gamma);
	return float4(color, 1.0);
}
