static const float PI = 3.141592653589793;
static const float EPSILON = 1.17549435E-38;

struct VertexIn
{
	float4 Position		: POSITION;
	float4 Normal		: NORMAL;
	float4 Tangent		: TANGENT;
	float4 Binormal		: BINORMAL;
	float2 UV			: TEXCOORD;
};

struct VertexOut
{
	float4 CameraPosition	: CAMERA_POSITION;
	float4 WorldPosition	: WORLD_POSITION;
	float4 Position			: SV_POSITION;
	float3x3 WorldNormal	: WORLD_NORMAL;
	float3 Normal			: NORMAL;
	float2 UV				: TEXCOORD;
};

struct Camera
{
	row_major float4x4 ViewProjection;
	float4 CameraPosition;
};

struct Properties
{
	row_major float4x4 Model;
};

ConstantBuffer<Camera> c_Camera : register(b0);
ConstantBuffer<Properties> c_Properties : register(b1);

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	output.CameraPosition = c_Camera.CameraPosition;
	output.WorldPosition = mul(v.Position, c_Properties.Model);
	output.Position = mul(output.WorldPosition, c_Camera.ViewProjection);

	float3 T = normalize(mul(c_Properties.Model, v.Tangent).xyz);
	float3 B = normalize(mul(c_Properties.Model, v.Binormal).xyz);
	output.Normal = normalize(mul(c_Properties.Model, v.Normal).xyz);
	output.WorldNormal = transpose(float3x3(T, B, output.Normal));

	output.UV = v.UV;

	return output;
}

struct PointLight
{
	float4 Position;			// xyz: position, w: radius
	float4 Color;				// rgb: color, a: intensity
};

struct DirectionalLight
{
	float4 Direction;
	float4 Color;				// rgb: color, a: intensity
};

StructuredBuffer<DirectionalLight> u_DirectionalLights : register (t0);
StructuredBuffer<PointLight> u_PointLights : register (t1);

Texture2D u_Albedo : register(t2);
Texture2D u_NormalMap : register(t3);

SamplerState u_Sampler : register(s0);

struct MaterialData
{
	float4 MRAO;
};

ConstantBuffer<MaterialData> c_Material : register(b2);

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

float LengthSq(const float3 v)
{
	return dot(v, v);
}

float3 Uncharted2Tonemap(const float3 x)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float4 PS_Main(VertexOut input) : SV_TARGET
{
	float4 albedo = u_Albedo.Sample(u_Sampler, input.UV);
	if (albedo.a < 0.05)
		discard;

	float3 tangentNormal = u_NormalMap.Sample(u_Sampler, input.UV).rgb * 2.0 - 1.0;
	float3 normal = normalize(mul(input.WorldNormal, tangentNormal));
	float metalness = c_Material.MRAO.r;
	float roughness = c_Material.MRAO.g;

	float3 view = normalize(input.CameraPosition.xyz - input.WorldPosition.xyz);
	float NdotV = max(dot(normal, view), 0.0);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo.rgb, metalness);

	float a2 = roughness * roughness;
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;

	float3 Lo = float3(0.0, 0.0, 0.0);

	// Lighting--------------------------------------------------------------------------------------------------
	// index 0 is reserved to check if data is present in Structured buffer or not.
	// Fixes high GPU usage on AMD cards when nothing is bound

	// Directional Light
	uint lightCount;
	uint lightStride;
	u_DirectionalLights.GetDimensions(lightCount, lightStride);
	for (uint dlIndex = 1; dlIndex < lightStride; ++dlIndex)
	{
		DirectionalLight light = u_DirectionalLights.Load(dlIndex);
		float3 L = -normalize(light.Direction.xyz);
		float NdotL = max(dot(normal, L), 0.0);

		float3 radiance = light.Color.rgb * light.Color.a;

		float3 H = normalize(L + view);
		float NDF = DistributionGGX(normal, H, a2);
		float G = GeometrySmith(NdotL, clamp(NdotV, 0.0, 1.0), k);
		float3 F = FresnelSchlickRoughness(clamp(dot(H, view), 0.0, 1.0), F0, roughness);

		float3 numerator = NDF * G * F;
		float denom = max(4.0 * NdotV * NdotL, 0.0001);
		float3 specular = numerator / denom;

		float3 kD = (1.0 - F) * (1.0 - metalness);
		Lo += (kD * (albedo.rgb / PI) + specular) * radiance * NdotL;
	}

	// Point Lights
	u_PointLights.GetDimensions(lightCount, lightStride);
	for (uint plIndex = 1; plIndex < lightStride; ++plIndex)
	{
		PointLight light = u_PointLights.Load(plIndex);
		float3 L = normalize(light.Position.xyz - input.WorldPosition.xyz);
		float NdotL = max(dot(normal, L), 0.0);
		float lightDistance2 = LengthSq(light.Position.xyz - input.WorldPosition.xyz);

		float lightRadius2 = light.Position.w * light.Position.w;
		float attenuation = saturate(1 - ((lightDistance2 * lightDistance2) / (lightRadius2 * lightRadius2)));
		attenuation = (attenuation * attenuation) / (lightDistance2 + 1.0);

		float3 radiance = light.Color.rgb * light.Color.a * attenuation;

		float3 H = normalize(L + view);
		float NDF = DistributionGGX(normal, H, a2);
		float G = GeometrySmith(NdotL, clamp(NdotV, 0.0, 1.0), k);
		float3 F = FresnelSchlickRoughness(clamp(dot(H, view), 0.0, 1.0), F0, roughness);

		float3 numerator = NDF * G * F;
		float denom = max(4.0 * NdotV * NdotL, 0.0001);
		float3 specular = numerator / denom;

		float3 kD = (1.0 - F) * (1.0 - metalness);
		Lo += (kD * (albedo.rgb / PI) + specular) * radiance * NdotL;
	}
	//-----------------------------------------------------------------------------------------------------------

	float ambient = 0.0f;

	float3 color = Lo + ambient;

	// Uncharted 2 Tonemapping
	const float W = 11.2;
	const float exposureBias = 2.0;
	float3 curr = Uncharted2Tonemap(exposureBias * color);
	float3 whiteScale = 1.0 / Uncharted2Tonemap(float3(W, W, W));

	color = curr * whiteScale;

	return float4(color, 1.0);
}
