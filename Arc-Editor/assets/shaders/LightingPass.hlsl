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
TextureCube<float4> IrradianceMap : register(t5, space1);

cbuffer Properties : register(b0, space1)
{
	float IrradianceIntensity;
	float EnvironmentRotation;
}

RWTexture2DArray<float4> OutputTexture : register(u0);

struct Params
{
	float3 Albedo;
	float Roughness;
	float Metalness;

	float3 WorldPos;
	float3 Normal;
	float3 View;
	float NdotV;
};

float3 RotateVectorAboutY(float angle, float3 vec)
{
	angle = radians(angle);
	float3x3 rotationMatrix =
	{
		float3(cos(angle), 0.0, sin(angle)),
		float3(0.0, 1.0, 0.0),
		float3(-sin(angle), 0.0, cos(angle))
	};
	return mul(rotationMatrix, vec);
}

float3 IBL(float3 F0, Params p)
{
	float NoV = clamp(p.NdotV, 0.0, 1.0);
	float3 F = FresnelSchlickRoughness(NoV, F0, p.Roughness);
	float3 kd = (1.0 - F) * (1.0 - p.Metalness);

	float3 irradiance = IrradianceMap.Sample(Sampler, RotateVectorAboutY(EnvironmentRotation, p.Normal)).rgb;
	float3 diffuseIBL = p.Albedo * irradiance;

	/*
	int envRadianceTexLevels = textureQueryLevels(u_RadianceMap);
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;
	vec3 specularIrradiance = textureLod(u_RadianceMap, RotateVectorAboutY(u_EnvironmentRotation, Lr), m_Params.Roughness * envRadianceTexLevels).rgb;
*/
	//vec2 specularBRDF = texture(u_BRDFLutMap, vec2(NoV, m_Params.Roughness)).rg;
	//vec3 specularIBL = specularIrradiance * (F); //* specularBRDF.x + specularBRDF.y);
	
	return (kd * diffuseIBL /*+ specularIBL*/) * IrradianceIntensity;
}


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
	
	
	Params params;
	params.Albedo = albedo.rgb;
	params.Roughness = Roughness;
	params.Metalness = Metalness;
	params.WorldPos = worldPosition;
	params.Normal = normal;
	params.View = view;
	params.NdotV = NdotV;
	

	float3 F0 = float3(0.04, 0.04, 0.04);
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

	float3 ambient = IBL(F0, params) * mra.b;
	float3 color = Lo + ambient + (emission.rgb);

	return float4(color, 1.0);
}
