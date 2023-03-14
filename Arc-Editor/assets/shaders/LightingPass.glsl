#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;

    vec3 u_CameraPosition;
};

layout(location = 0) out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_FragColor;

const float PI = 3.141592653589793;
const float EPSILON = 1.17549435E-38;

const int MAX_NUM_LIGHTS = 200;
const int MAX_NUM_DIR_LIGHTS = 3;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;

    vec3 u_CameraPosition;
};

struct PointLight
{
    vec4 u_Position;

	/*
	* rgb: color, a: intensity
	*/
    vec4 u_Color;
    
    /* packed into a vec4
    x: range
    y: cutOffAngle
    z: outerCutOffAngle
    w: light type */
    vec4 u_AttenFactors;
    
    // Used for directional and spot lights
    vec4 u_LightDir;
};

layout (std140, binding = 1) uniform PointLightBuffer
{
    PointLight u_PointLights[MAX_NUM_LIGHTS];
};

struct DirectionalLight
{
    vec4 u_Position;

	/*
	* rgb: color, a: intensity
	*/
    vec4 u_Color;
    
    // Used for directional and spot lights
    vec4 u_LightDir;
	
	mat4 u_DirLightViewProj;
};

layout (std140, binding = 2) uniform DirectionalLightBuffer
{
    DirectionalLight u_DirectionalLights[MAX_NUM_DIR_LIGHTS];
};

//Employ stochastic sampling
const int PCF_SAMPLES = 64;
const vec2 gPoissonDisk[64] = vec2[](
	vec2(-0.884081, 0.124488),
	vec2(-0.714377, 0.027940),
	vec2(-0.747945, 0.227922),
	vec2(-0.939609, 0.243634),
	vec2(-0.985465, 0.045534),
	vec2(-0.861367, -0.136222),
	vec2(-0.881934, 0.396908),
	vec2(-0.466938, 0.014526),
	vec2(-0.558207, 0.212662),
	vec2(-0.578447, -0.095822),
	vec2(-0.740266, -0.095631),
	vec2(-0.751681, 0.472604),
	vec2(-0.553147, -0.243177),
	vec2(-0.674762, -0.330730),
	vec2(-0.402765, -0.122087),
	vec2(-0.319776, -0.312166),
	vec2(-0.413923, -0.439757),
	vec2(-0.979153, -0.201245),
	vec2(-0.865579, -0.288695),
	vec2(-0.243704, -0.186378),
	vec2(-0.294920, -0.055748),
	vec2(-0.604452, -0.544251),
	vec2(-0.418056, -0.587679),
	vec2(-0.549156, -0.415877),
	vec2(-0.238080, -0.611761),
	vec2(-0.267004, -0.459702),
	vec2(-0.100006, -0.229116),
	vec2(-0.101928, -0.380382),
	vec2(-0.681467, -0.700773),
	vec2(-0.763488, -0.543386),
	vec2(-0.549030, -0.750749),
	vec2(-0.809045, -0.408738),
	vec2(-0.388134, -0.773448),
	vec2(-0.429392, -0.894892),
	vec2(-0.131597, 0.065058),
	vec2(-0.275002, 0.102922),
	vec2(-0.106117, -0.068327),
	vec2(-0.294586, -0.891515),
	vec2(-0.629418, 0.379387),
	vec2(-0.407257, 0.339748),
	vec2(0.071650, -0.384284),
	vec2(0.022018, -0.263793),
	vec2(0.003879, -0.136073),
	vec2(-0.137533, -0.767844),
	vec2(-0.050874, -0.906068),
	vec2(0.114133, -0.070053),
	vec2(0.163314, -0.217231),
	vec2(-0.100262, -0.587992),
	vec2(-0.004942, 0.125368),
	vec2(0.035302, -0.619310),
	vec2(0.195646, -0.459022),
	vec2(0.303969, -0.346362),
	vec2(-0.678118, 0.685099),
	vec2(-0.628418, 0.507978),
	vec2(-0.508473, 0.458753),
	vec2(0.032134, -0.782030),
	vec2(0.122595, 0.280353),
	vec2(-0.043643, 0.312119),
	vec2(0.132993, 0.085170),
	vec2(-0.192106, 0.285848),
	vec2(0.183621, -0.713242),
	vec2(0.265220, -0.596716),
	vec2(-0.009628, -0.483058),
	vec2(-0.018516, 0.435703)
);

uniform sampler2D u_Albedo;
uniform sampler2D u_Normal;
uniform sampler2D u_MetallicRoughnessAO;
uniform sampler2D u_Emission;
uniform sampler2D u_Depth;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_RadianceMap;
uniform sampler2D u_BRDFLutMap;
uniform sampler2D u_DirectionalShadowMap[MAX_NUM_DIR_LIGHTS];

uniform float u_IrradianceIntensity;
uniform float u_EnvironmentRotation;

uniform int u_NumPointLights;
uniform int u_NumDirectionalLights;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 WorldPos;
	vec3 Normal;
	vec3 View;
	float NdotV;
};

PBRParameters m_Params;

// N: Normal, H: Halfway, a2: pow(roughness, 2)
float DistributionGGX(const vec3 N, const vec3 H, const float a2)
{
    float NdotH = max(dot(N, H), 0.0);
	float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

// N: Normal, V: View, L: Light, k: (roughness + 1)^2 / 8.0
float GeometrySmith(const float NdotL, const float NdotV, const float k)
{
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

    return ggx1 * ggx2;
}

vec3 Fresnel(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float CalculateHardShadows(int shadowmapIndex, vec3 shadowCoords, float bias)
{
	float z = texture(u_DirectionalShadowMap[shadowmapIndex], shadowCoords.xy).r;
	return shadowCoords.z - bias > z ? 0.0 : 1.0;
}

float CalculateSoftShadows(int shadowmapIndex, vec3 shadowCoords, float bias)
{
	vec2 texelSize = 1.0 / textureSize(u_DirectionalShadowMap[shadowmapIndex], 0);
	int sampleCount = 1;
	float shadow = 0.0;
	for(int x = -sampleCount; x <= sampleCount; ++x)
	{
		for(int y = -sampleCount; y <= sampleCount; ++y)
		{
			float z = texture(u_DirectionalShadowMap[shadowmapIndex], shadowCoords.xy + vec2(x, y) * texelSize).r;
			shadow += shadowCoords.z - bias > z ? 0.0 : 1.0;
		}
	}
    float tmp = sampleCount * 2 + 1;
	shadow /= (tmp * tmp);
	return shadow;
}

float CalculatePCSS(int shadowmapIndex, vec3 shadowCoords, float bias)
{
	vec2 texelSizeMultiplier = 3.0 / textureSize(u_DirectionalShadowMap[shadowmapIndex], 0);
	float shadow = 0.0;
	for (int i = 0; i < PCF_SAMPLES; i++)
	{
		vec2 offset = gPoissonDisk[i] * texelSizeMultiplier;
		float z = texture(u_DirectionalShadowMap[shadowmapIndex], shadowCoords.xy + offset).r;
		shadow += shadowCoords.z - bias > z ? 0.0 : 1.0;
	}
	return shadow / float(PCF_SAMPLES);
}

float CalcDirectionalShadowFactor(DirectionalLight directionalLight, const float NdotL, int shadowmapIndex)
{
	vec4 dirLightViewProj = directionalLight.u_DirLightViewProj * vec4(m_Params.WorldPos, 1);
	vec3 projCoords = dirLightViewProj.xyz / dirLightViewProj.w;
	projCoords = (projCoords * 0.5) + 0.5;
	
	if(projCoords.z > 1.0)
        return 0.0;

	float bias = max(0.0008 * (1.0 - NdotL), 0.0008);
	int shadowQuality = int(directionalLight.u_Position.w);
	switch (shadowQuality)
	{
		case 0: return CalculateHardShadows(shadowmapIndex, projCoords, bias);
		case 1: return CalculateSoftShadows(shadowmapIndex, projCoords, bias);
		case 2: return CalculatePCSS(shadowmapIndex, projCoords, bias);
	}
}

float LengthSq(const vec3 v)
{
	return dot(v, v);
}

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
	angle = radians(angle);
	mat3x3 rotationMatrix = { vec3(cos(angle),0.0,sin(angle)),
							vec3(0.0,1.0,0.0),
							vec3(-sin(angle),0.0,cos(angle)) };
	return rotationMatrix * vec;
}

vec3 IBL(vec3 F0)
{
	float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
	vec3 F = FresnelSchlickRoughness(NoV, F0, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);

	vec3 irradiance = texture(u_IrradianceMap, RotateVectorAboutY(u_EnvironmentRotation, m_Params.Normal)).rgb;
	vec3 diffuseIBL = m_Params.Albedo * irradiance;

	int envRadianceTexLevels = textureQueryLevels(u_RadianceMap);
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;
	vec3 specularIrradiance = textureLod(u_RadianceMap, RotateVectorAboutY(u_EnvironmentRotation, Lr), m_Params.Roughness * envRadianceTexLevels).rgb;

	vec2 specularBRDF = texture(u_BRDFLutMap, vec2(NoV, m_Params.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F); //* specularBRDF.x + specularBRDF.y);
	
	return (kd * diffuseIBL + specularIBL) * u_IrradianceIntensity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///// Reconstruct position from depth and view/projection //////////////////////////////////////////
//// https://gamedev.stackexchange.com/questions/108856/fast-position-reconstruction-from-depth ////
////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 WorldPosFromDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	vec4 clipSpace = vec4(v_TexCoord * 2.0 - 1.0, z, 1.0);
	vec4 viewSpace = inverse(u_Projection) * clipSpace;
	viewSpace /= viewSpace.w;
	vec4 worldSpace = inverse(u_View) * viewSpace;
	return worldSpace.xyz;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///// Octahedron-normal vectors ////////////////////////////////////////////////////////////////////
//// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/ ////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
vec2 signNotZero( vec2 v )
{
	return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

vec3 Decode( vec2 e )
{
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0)
		v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);

	return normalize(v);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Entry Point ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
	vec4 albedo = texture(u_Albedo, v_TexCoord);
	m_Params.Albedo = albedo.rgb;

	vec4 metallicRoughnessAO = texture(u_MetallicRoughnessAO, v_TexCoord);
	if (int(metallicRoughnessAO.a) == 0)
	{
		o_FragColor = albedo;
		return;
	}

	vec4 depth = texture(u_Depth, v_TexCoord);
	m_Params.WorldPos = WorldPosFromDepth(depth.r);
	m_Params.Normal = Decode(texture(u_Normal, v_TexCoord).rg);

	m_Params.Metalness = metallicRoughnessAO.r;
	m_Params.Roughness = metallicRoughnessAO.g;
	float ao = metallicRoughnessAO.b;

	vec4 emission = texture(u_Emission, v_TexCoord);

	m_Params.View = normalize(u_CameraPosition - m_Params.WorldPos);
	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, m_Params.Albedo, m_Params.Metalness);

	float a2 = m_Params.Roughness * m_Params.Roughness;
	float r = m_Params.Roughness + 1.0;
	float k = (r * r) / 8.0;

    // reflectance equation
    vec3 Lo = vec3(0.0);

	for (int i = 0; i < u_NumDirectionalLights; ++i)
	{
		DirectionalLight light = u_DirectionalLights[i];
		vec3 L = -1.0 * normalize(light.u_LightDir.xyz);
		float NdotL = max(dot(m_Params.Normal, L), 0.0);
		float shadow = CalcDirectionalShadowFactor(light, NdotL, i);

		if (shadow <= EPSILON)
			continue;

        vec3 radiance = shadow * light.u_Color.rgb * light.u_Color.a;

        // Cook-Torrance BRDF
        vec3 H = normalize(L + m_Params.View);
        float NDF = DistributionGGX(m_Params.Normal, H, a2);
        float G = GeometrySmith(NdotL, clamp(m_Params.NdotV, 0.0, 1.0), k);
        vec3 F = FresnelSchlickRoughness(clamp(dot(H, m_Params.View), 0.0, 1.0), F0, m_Params.Roughness);

        vec3 numerator = NDF * G * F;
        float denom = max(4.0 * m_Params.NdotV * NdotL, 0.0001);
        vec3 specular = numerator / denom;

        vec3 kD = (1.0 - F) * (1.0 - m_Params.Metalness);
        Lo += (kD * (m_Params.Albedo / PI) + specular) * radiance * NdotL;
	}

    for (int i = 0; i < u_NumPointLights; ++i)
    {
		PointLight light = u_PointLights[i];
        uint type = uint(round(light.u_AttenFactors.w));

        vec3 L;
		float NdotL;
		float shadow = 1.0;
        float attenuation = 1.0;
        switch (type)
        {
			// Point Light
			case 1:
			{
				L = normalize(light.u_Position.rgb - m_Params.WorldPos);
				NdotL = max(dot(m_Params.Normal, L), 0.0);
				vec4 attenFactor = light.u_AttenFactors;
				float lightDistance2 = LengthSq(light.u_Position.xyz - m_Params.WorldPos);
				float lightRadius2 = attenFactor.x * attenFactor.x;
				attenuation = clamp(1 - ((lightDistance2 * lightDistance2) / (lightRadius2 * lightRadius2)), 0.0, 1.0);
				attenuation = (attenuation * attenuation) / (lightDistance2 + 1.0);
				break;
			}
			
			// Spot Light
			case 2:
			{
				L = normalize(light.u_Position.rgb - m_Params.WorldPos);
				NdotL = max(dot(m_Params.Normal, L), 0.0);
				vec4 attenFactor = light.u_AttenFactors;
				float lightDistance2 = LengthSq(light.u_Position.xyz - m_Params.WorldPos);
				float lightRadius2 = attenFactor.x * attenFactor.x;
				if (lightRadius2 > lightDistance2)
				{
					float theta = dot(L, normalize(-light.u_LightDir.xyz));
					float epsilon = attenFactor.y - attenFactor.z;
					float intensity = clamp((theta - attenFactor.z) / epsilon, 0.0, 1.0);
					attenuation = intensity / lightDistance2;
				}
				else
				{
					attenuation = 0.0;
				}
				break;
			}
        }

		if (shadow <= EPSILON)
			continue;

        vec3 radiance = shadow * light.u_Color.rgb * light.u_Color.a * attenuation;

        // Cook-Torrance BRDF
        vec3 H = normalize(L + m_Params.View);
        float NDF = DistributionGGX(m_Params.Normal, H, a2);
        float G = GeometrySmith(NdotL, clamp(m_Params.NdotV, 0.0, 1.0), k);
        vec3 F = FresnelSchlickRoughness(clamp(dot(H, m_Params.View), 0.0, 1.0), F0, m_Params.Roughness);

        vec3 numerator = NDF * G * F;
        float denom = max(4.0 * m_Params.NdotV * NdotL, 0.0001);
        vec3 specular = numerator / denom;

        vec3 kD = (1.0 - F) * (1.0 - m_Params.Metalness);
        Lo += (kD * (m_Params.Albedo / PI) + specular) * radiance * NdotL;
    }

	vec3 ambient = IBL(F0) * ao;
	vec3 result = Lo + ambient + (emission.rgb * emission.a * 255);

	o_FragColor = vec4(result, 1);
}
