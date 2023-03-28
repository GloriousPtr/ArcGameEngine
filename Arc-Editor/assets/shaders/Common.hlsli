#ifndef COMMON_HEADER
#define COMMON_HEADER

#include "MathsConstants.hlsli"

// Root constant
cbuffer Transform : register(b0, space0)
{
	float4x4 Model;
}

cbuffer GlobalData : register(b1, space0)
{
	float4x4 CameraView;
	float4x4 CameraProjection;
	float4x4 CameraViewProjection;
	float4 CameraPosition;
	uint NumDirectionalLights = 0;
	uint NumPointLights = 0;
	uint NumSpotLights = 0;
}

SamplerState Sampler : register(s0);

struct PointLight
{
	float4 Position;			// xyz: position, w: radius
	float4 Color;				// rgb: color, a: intensity
};

StructuredBuffer<PointLight> PointLights : register (t0, space0);




// N: Normal, H: Halfway, a2: pow(roughness, 2)
float DistributionGGX(const float3 N, const float3 H, const float a2)
{
	float NdotH = max(dot(N, H), 0.0);
	float denom = mul(mul(NdotH, NdotH), (a2 - 1.0)) + 1.0;
	return a2 / mul(mul(PI, denom), denom);
}

// N: Normal, V: View, L: Light, k: (roughness + 1)^2 / 8.0
float GeometrySmith(const float NdotL, const float NdotV, const float k)
{
	float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

	return ggx1 * ggx2;
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float minusR = 1.0 - roughness;
	return F0 + (max(float3(minusR, minusR, minusR), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

#endif
