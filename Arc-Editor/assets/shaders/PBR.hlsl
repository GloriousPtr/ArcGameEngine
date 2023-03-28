#include "MathsConstants.hlsli"
#include "Common.hlsli"

struct VertexIn
{
	float3 Position			: POSITION;
	float2 UV				: TEXCOORD;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Bitangent		: BITANGENT;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float2 UV				: TEXCOORD;
	float4 WorldPosition	: WORLD_POSITION;
	float4 WorldNormal		: WORLD_NORMAL;
	float3x3 TBN			: TANGENT_BASIS;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	output.WorldPosition = mul(Model, float4(v.Position, 1.0));
	output.WorldNormal = mul(Model, float4(v.Normal, 1.0));
	output.Position = mul(CameraViewProjection, output.WorldPosition);
	output.UV = v.UV;
	output.TBN = float3x3(v.Tangent, v.Bitangent, v.Normal);

	return output;
}

cbuffer Textures : register(b0, space1)
{
	uint AlbedoTexture;
	uint NormalTexture;
}

cbuffer MaterialProperties : register(b1, space1)
{
	float4 AlbedoColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	float Roughness = 1.0f;
	float Metalness = 0.0f;
	float AlphaCutoffThreshold = 0.01f;
}

float4 PS_Main(VertexOut input) : SV_TARGET
{
	const Texture2D<float4> albedoTexture = ResourceDescriptorHeap[AlbedoTexture];
	const Texture2D<float4> normalTexture = ResourceDescriptorHeap[NormalTexture];

	float4 albedo = albedoTexture.Sample(Sampler, input.UV);

	albedo *= AlbedoColor;

	//float3 normal = normalize(mul(input.TBN, normalTexture.Sample(Sampler, input.UV).rgb * 2.0f - 1.0f));
	float3 normal = normalize(input.WorldNormal).xyz;

	float3 view = normalize(CameraPosition.xyz - input.WorldPosition.xyz);
	float NdotV = max(dot(normal, view), 0.0);

	float3 F0 = float3(0.4, 0.4, 0.4);
	F0 = lerp(F0, albedo.rgb, Metalness);

	float a2 = Roughness * Roughness;
	float r = Roughness + 1.0;
	float k = (r * r) / 8.0;

	float3 Lo = float3(0.0, 0.0, 0.0);

	// Point Lights
	for (uint i = 0; i < NumPointLights; ++i)
	{
		PointLight light = PointLights.Load(i);

		float3 L = normalize(light.Position.xyz - input.WorldPosition.xyz);
		float NdotL = max(dot(normal, L), 0.0);
		float3 lightDistance = light.Position.xyz - input.WorldPosition.xyz;
		float lightDistanceSq = dot(lightDistance, lightDistance);
		float lightRadiusSq = light.Position.w * light.Position.w;
		float attenuation = saturate(1 - ((lightDistanceSq * lightDistanceSq) / (lightRadiusSq * lightRadiusSq)));
		attenuation = (attenuation * attenuation) / (lightDistanceSq + 1.0);

		float3 radiance = light.Color.rgb * light.Color.a * attenuation;

		float3 H = normalize(L + view);
		float NDF = DistributionGGX(normal, H, a2);
		float G = GeometrySmith(NdotL, clamp(NdotV, 0.0, 1.0), k);
		float3 F = FresnelSchlickRoughness(clamp(dot(H, view), 0.0, 1.0), F0, Roughness);

		float3 numerator = NDF * G * F;
		float denom = max(4.0 * NdotV * NdotL, 0.0001);
		float3 specular = numerator / denom;
		float3 kd = (1.0 - F) * (1.0 - Metalness);
		Lo += ((kd * albedo.rgb / PI) + specular) * radiance * NdotL;
	}

	float ambient = 0.0f;
	float3 color = Lo + ambient;

	return float4(color, 1.0);
}
