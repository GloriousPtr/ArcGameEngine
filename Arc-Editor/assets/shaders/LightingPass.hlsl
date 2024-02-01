#include "Common.hlsli"

struct VertexIn
{
	float4 Position			: POSITION;
	float2 UV				: TEXCOORD;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float2 UV				: TEXCOORD;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;
	output.Position = v.Position;
	output.UV = v.UV;
	return output;
}

Texture2D Albedo : register(t0, space1);
Texture2D Normal : register(t1, space1);
Texture2D MetalicRoughnessAO : register(t2, space1);
Texture2D Emission : register(t3, space1);
Texture2D<float> Depth : register(t4, space1);

RWTexture2DArray<float4> OutputTexture : register(u0);

float4 PS_Main(VertexOut input) : SV_TARGET
{
	float2 uv = input.UV;

	float4 albedo = Albedo.Sample(Sampler, uv);
	float3 normal = Normal.Sample(Sampler, uv).xyz;
	float4 mra = MetalicRoughnessAO.Sample(Sampler, uv);
	float4 emission = Emission.Sample(Sampler, uv);
	if (emission.r > 0.1f)
		emission = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float depth = Depth.Sample(Sampler, uv);
	float3 worldPosition = WorldPosFromDepth(uv, depth);
	float Metalness = mra.r;
	float Roughness = mra.g;

	float3 view = normalize(CameraPosition.xyz - worldPosition);
	float NdotV = max(dot(normal, view), 0.0);

	float3 F0 = float3(0.4, 0.4, 0.4);
	F0 = lerp(F0, albedo.rgb, Metalness);

	float a2 = Roughness * Roughness;
	float r = Roughness + 1.0;
	float k = (r * r) / 8.0;

	float3 Lo = float3(0.0, 0.0, 0.0);

	// Directional Lights
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

		float3 L = normalize(light.Position.xyz - worldPosition);
		float NdotL = max(dot(normal, L), 0.0);
		float3 lightDistance = light.Position.xyz - worldPosition;
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

		float3 L = normalize(light.Position.xyz - worldPosition);
		float NdotL = max(dot(normal, L), 0.0f);
		float3 lightDistance = light.Position.xyz - worldPosition;
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
	float3 color = Lo + ambient + (emission.rgb);

	return float4(color, 1.0);
}
