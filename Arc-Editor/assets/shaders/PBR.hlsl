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
	float3 WorldPosition	: WORLD_POSITION;
	float3 WorldNormal		: WORLD_NORMAL;
	float3x3 TBN			: TANGENT_BASIS;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	float4 worldPosition = mul(Model, float4(v.Position, 1.0));
	output.Position = mul(CameraViewProjection, worldPosition);
	output.UV = v.UV;
	output.WorldPosition = worldPosition.xyz;
	output.WorldNormal = normalize(mul(Model, float4(v.Normal, 1.0)).xyz);
	float3 T = normalize(mul(Model, float4(v.Tangent, 1.0)).xyz);
	float3 B = normalize(mul(Model, float4(v.Bitangent, 1.0)).xyz);
	output.TBN = float3x3(T, B, output.WorldNormal);

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

	float3 normal = normalize(mul(input.TBN, normalTexture.Sample(Sampler, input.UV).rgb * 2.0f - 1.0f));
	//float3 normal = normalize(input.WorldNormal);

	float3 view = normalize(CameraPosition.xyz - input.WorldPosition);
	float NdotV = max(dot(normal, view), 0.0);

	float3 F0 = float3(0.4, 0.4, 0.4);
	F0 = lerp(F0, albedo.rgb, Metalness);

	float a2 = Roughness * Roughness;
	float r = Roughness + 1.0;
	float k = (r * r) / 8.0;

	float3 Lo = float3(0.0, 0.0, 0.0);

	for (uint dirLightIndex = 0; dirLightIndex < NumDirectionalLights; ++dirLightIndex)
	{
		DirectionalLight light = DirectionalLights.Load(dirLightIndex);

		float3 L = -normalize(light.Direction.xyz);
		float NdotL = max(dot(normal, L), 0.0f);
		float3 radiance = light.Color.rgb * light.Color.a;

		float3 H = normalize(L + view);
		float NDF = DistributionGGX(normal, H, a2);
		float G = GeometrySmith(NdotL, clamp(NdotV, 0.0f, 1.0f), k);
		float3 F = FresnelSchlickRoughness(clamp(dot(H, view), 0.0f, 1.0f), F0, Roughness);

		float3 numerator = NDF * G * F;
		float denom = max(4.0f * NdotV * NdotL, 0.0001);
		float3 specular = numerator / denom;
		float3 kd = (1.0f - F) * (1.0f - Metalness);
		Lo += (kd * (albedo.rgb / PI) + specular) * radiance * NdotL;
	}

	// Point Lights
	for (uint pointLightIndex = 0; pointLightIndex < NumPointLights; ++pointLightIndex)
	{
		PointLight light = PointLights.Load(pointLightIndex);

		float3 L = normalize(light.Position.xyz - input.WorldPosition);
		float NdotL = max(dot(normal, L), 0.0);
		float3 lightDistance = light.Position.xyz - input.WorldPosition;
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
		Lo += (kd * (albedo.rgb / PI) + specular) * radiance * NdotL;
	}

	// Spot Lights
	for (uint spotLightIndex = 0; spotLightIndex < NumSpotLights; ++spotLightIndex)
	{
		SpotLight light = SpotLights.Load(spotLightIndex);

		float3 L = normalize(light.Position.xyz - input.WorldPosition);
		float NdotL = max(dot(normal, L), 0.0f);
		float3 lightDistance = light.Position.xyz - input.WorldPosition;
		float lightDistanceSq = dot(lightDistance, lightDistance);
		float lightRadiusSq = light.Position.w * light.Position.w;

		float attenuation = 0.0f;
		if (lightRadiusSq > lightDistanceSq)
		{
			float theta = dot(L, normalize(-light.Direction.xyz));
			float epsilon = light.AttenuationFactors.x - light.AttenuationFactors.y;
			float intensity = clamp((theta - light.AttenuationFactors.y) / epsilon, 0.0f, 1.0f);
			attenuation = intensity / lightDistanceSq;
		}

		float3 radiance = light.Color.rgb * light.Color.a * attenuation;

		float3 H = normalize(L + view);
		float NDF = DistributionGGX(normal, H, a2);
		float G = GeometrySmith(NdotL, clamp(NdotV, 0.0, 1.0), k);
		float3 F = FresnelSchlickRoughness(clamp(dot(H, view), 0.0, 1.0), F0, Roughness);

		float3 numerator = NDF * G * F;
		float denom = max(4.0 * NdotV * NdotL, 0.0001);
		float3 specular = numerator / denom;
		float3 kd = (1.0 - F) * (1.0 - Metalness);
		Lo += (kd * (albedo.rgb / PI) + specular) * radiance * NdotL;
	}

	float3 ambient = albedo.rgb * 0.5f;
	float3 color = Lo + ambient;

	return float4(color, 1.0);
}
