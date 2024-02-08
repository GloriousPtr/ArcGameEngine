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

cbuffer Properties : register(b0, space1)
{
	uint AlbedoTexture;
	uint NormalTexture;
	uint MRATexture;
	uint EmissiveTexture;
	uint DepthTexture;
	uint IrradianceTexture;
	
	float IrradianceIntensity;
	float EnvironmentRotation;
}

RWTexture2DArray<float4> OutputTexture : register(u0);

//Employ stochastic sampling
static const int PCF_SAMPLES = 64;
static const float2 gPoissonDisk[64] =
{
	float2(-0.884081, 0.124488),
	float2(-0.714377, 0.027940),
	float2(-0.747945, 0.227922),
	float2(-0.939609, 0.243634),
	float2(-0.985465, 0.045534),
	float2(-0.861367, -0.136222),
	float2(-0.881934, 0.396908),
	float2(-0.466938, 0.014526),
	float2(-0.558207, 0.212662),
	float2(-0.578447, -0.095822),
	float2(-0.740266, -0.095631),
	float2(-0.751681, 0.472604),
	float2(-0.553147, -0.243177),
	float2(-0.674762, -0.330730),
	float2(-0.402765, -0.122087),
	float2(-0.319776, -0.312166),
	float2(-0.413923, -0.439757),
	float2(-0.979153, -0.201245),
	float2(-0.865579, -0.288695),
	float2(-0.243704, -0.186378),
	float2(-0.294920, -0.055748),
	float2(-0.604452, -0.544251),
	float2(-0.418056, -0.587679),
	float2(-0.549156, -0.415877),
	float2(-0.238080, -0.611761),
	float2(-0.267004, -0.459702),
	float2(-0.100006, -0.229116),
	float2(-0.101928, -0.380382),
	float2(-0.681467, -0.700773),
	float2(-0.763488, -0.543386),
	float2(-0.549030, -0.750749),
	float2(-0.809045, -0.408738),
	float2(-0.388134, -0.773448),
	float2(-0.429392, -0.894892),
	float2(-0.131597, 0.065058),
	float2(-0.275002, 0.102922),
	float2(-0.106117, -0.068327),
	float2(-0.294586, -0.891515),
	float2(-0.629418, 0.379387),
	float2(-0.407257, 0.339748),
	float2(0.071650, -0.384284),
	float2(0.022018, -0.263793),
	float2(0.003879, -0.136073),
	float2(-0.137533, -0.767844),
	float2(-0.050874, -0.906068),
	float2(0.114133, -0.070053),
	float2(0.163314, -0.217231),
	float2(-0.100262, -0.587992),
	float2(-0.004942, 0.125368),
	float2(0.035302, -0.619310),
	float2(0.195646, -0.459022),
	float2(0.303969, -0.346362),
	float2(-0.678118, 0.685099),
	float2(-0.628418, 0.507978),
	float2(-0.508473, 0.458753),
	float2(0.032134, -0.782030),
	float2(0.122595, 0.280353),
	float2(-0.043643, 0.312119),
	float2(0.132993, 0.085170),
	float2(-0.192106, 0.285848),
	float2(0.183621, -0.713242),
	float2(0.265220, -0.596716),
	float2(-0.009628, -0.483058),
	float2(-0.018516, 0.435703)
};

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
	TextureCube<float4> IrradianceMap = ResourceDescriptorHeap[IrradianceTexture];
	
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





float CalculateHardShadows(Texture2D<float> shadowMap, float3 shadowCoords, float bias)
{
	float z = shadowMap.Sample(PointClampSampler, shadowCoords.xy);
	return shadowCoords.z - bias > z ? 0.0 : 1.0;
}

float CalculateSoftShadows(Texture2D<float> shadowMap, float3 shadowCoords, float bias)
{
	float textureWidth, textureHeight;
	shadowMap.GetDimensions(textureWidth, textureHeight);
	float2 texelSize = float2(1.0, 1.0) / float2(textureWidth, textureHeight);
	int sampleCount = 1;
	float shadow = 0.0;
	for (int x = -sampleCount; x <= sampleCount; ++x)
	{
		for (int y = -sampleCount; y <= sampleCount; ++y)
		{
			float z = shadowMap.Sample(PointClampSampler, shadowCoords.xy + float2(x, y) * texelSize).r;
			shadow += shadowCoords.z - bias > z ? 0.0 : 1.0;
		}
	}
	float tmp = sampleCount * 2 + 1;
	shadow /= (tmp * tmp);
	return shadow;
}

float CalculatePCSS(Texture2D<float> shadowMap, float3 shadowCoords, float bias)
{
	float textureWidth, textureHeight;
	shadowMap.GetDimensions(textureWidth, textureHeight);
	float2 texelSizeMultiplier = float2(3.0, 3.0) / float2(textureWidth, textureHeight);
	float shadow = 0.0;
	for (int i = 0; i < PCF_SAMPLES; i++)
	{
		float2 offset = gPoissonDisk[i] * texelSizeMultiplier;
		float z = shadowMap.Sample(PointClampSampler, shadowCoords.xy + offset).r;
		shadow += shadowCoords.z - bias > z ? 0.0 : 1.0;
	}
	return shadow / float(PCF_SAMPLES);
}

float CalcDirectionalShadowFactor(Params params, DirectionalLight directionalLight, const float NdotL)
{
	float4 dirLightViewProj = mul(directionalLight.ViewProj, float4(params.WorldPos, 1));
	float3 projCoords = dirLightViewProj.xyz / dirLightViewProj.w;
	
	if (projCoords.z > 1.0)
		return 0.0f;
	
	projCoords.x = (projCoords.x * 0.5) + 0.5;
	projCoords.y = (projCoords.y * -0.5) + 0.5;
	
	float b = directionalLight.QualityTextureBias.z;
	float bias = max(b * (1.0f - NdotL), 0.1f * b);
	int shadowQuality = int(directionalLight.QualityTextureBias.x);
	Texture2D<float> shadowMap = ResourceDescriptorHeap[directionalLight.QualityTextureBias.y];
	switch (shadowQuality)
	{
		case 0:
			return CalculateHardShadows(shadowMap, projCoords, bias);
		case 1:
			return CalculateSoftShadows(shadowMap, projCoords, bias);
		case 2:
			return CalculatePCSS(shadowMap, projCoords, bias);
	}

	return 0.0f;
}






float4 PS_Main(VertexOut input) : SV_TARGET
{
	float2 uv = input.UV;

	Texture2D Albedo = ResourceDescriptorHeap[AlbedoTexture];
	Texture2D Normal = ResourceDescriptorHeap[NormalTexture];
	Texture2D MetalicRoughnessAO = ResourceDescriptorHeap[MRATexture];
	Texture2D Emission = ResourceDescriptorHeap[EmissiveTexture];
	Texture2D<float> Depth = ResourceDescriptorHeap[DepthTexture];
	
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

		float3 L = -1.0 * normalize(light.Direction.xyz);
		float NdotL = max(dot(normal, L), 0.0);
		float shadow = CalcDirectionalShadowFactor(params, light, NdotL);

		float3 radiance = shadow * light.Color.rgb * light.Color.a;

        // Cook-Torrance BRDF
		float3 H = normalize(L + view);
		float NDF = DistributionGGX(normal, H, a2);
		float G = GeometrySmith(NdotL, clamp(NdotV, 0.0, 1.0), k);
		float3 F = FresnelSchlickRoughness(clamp(dot(H, view), 0.0, 1.0), F0, Roughness);

		float3 numerator = NDF * G * F;
		float denom = max(4.0 * NdotV * NdotL, 0.0001);
		float3 specular = numerator / denom;

		float3 kD = (1.0 - F) * (1.0 - Metalness);
		Lo += (kD * (albedo.rgb / PI) + specular) * radiance * NdotL;
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
