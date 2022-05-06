#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;

    vec4 u_CameraPosition;
};

layout(location = 0) out vec2 v_TexCoord;
layout(location = 1) out vec3 v_CameraPosition;

void main()
{
	v_TexCoord = a_TexCoord;
    v_CameraPosition = u_CameraPosition.xyz;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_CameraPosition;

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

    vec4 u_CameraPosition;
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
    uint u_NumPointLights;
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

layout (std140, binding = 1) uniform DirectionalLightBuffer
{
    DirectionalLight u_DirectionalLights[MAX_NUM_DIR_LIGHTS];
    uint u_NumDirectionalLights;
};

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

float CalcDirectionalShadowFactor(DirectionalLight directionalLight, const float NdotL, int shadowmapIndex)
{
	vec4 dirLightViewProj = directionalLight.u_DirLightViewProj * vec4(m_Params.WorldPos, 1);
	vec3 projCoords = dirLightViewProj.xyz / dirLightViewProj.w;
	projCoords = (projCoords * 0.5) + 0.5;

	float currentDepth = projCoords.z;

	float bias = max(0.0008 * (1.0 - NdotL), 0.0008);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_DirectionalShadowMap[shadowmapIndex], 0);
	int sampleCount = 3;
	for(int x = -sampleCount; x <= sampleCount; ++x)
	{
		for(int y = -sampleCount; y <= sampleCount; ++y)
		{
			float pcfDepth = texture(u_DirectionalShadowMap[shadowmapIndex], projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
    float tmp = sampleCount * 2 + 1;
	shadow /= (tmp * tmp);

	if(projCoords.z > 1.0)
        shadow = 0.0;

	return shadow;
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

	int envRadianceTexLevels = textureQueryLevels(u_RadianceMap) - 5;
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;
	vec3 specularIrradiance = textureLod(u_RadianceMap, RotateVectorAboutY(u_EnvironmentRotation, Lr), m_Params.Roughness * envRadianceTexLevels).rgb;

	vec2 specularBRDF = texture(u_BRDFLutMap, vec2(1.0 - m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);
	
	return (kd * diffuseIBL + specularIBL) * u_IrradianceIntensity;
}

vec3 WorldPosFromDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	vec4 clipSpace = vec4(v_TexCoord * 2.0 - 1.0, z, 1.0);
	vec4 viewSpace = inverse(u_Projection) * clipSpace;
	viewSpace /= viewSpace.w;
	vec4 worldSpace = inverse(u_View) * viewSpace;
	return worldSpace.xyz;
}

void main()
{
	vec4 albedo = texture(u_Albedo, v_TexCoord);
	m_Params.Albedo = albedo.rgb;

	if (albedo.x == albedo.y
		&& albedo.y == albedo.z
		&& albedo.z == albedo.a
		&& albedo.a <= EPSILON)
		discard;

	vec4 depth = texture(u_Depth, v_TexCoord);
	m_Params.WorldPos = WorldPosFromDepth(depth.r); //texture(u_Position, v_TexCoord).rgb;
	m_Params.Normal = texture(u_Normal, v_TexCoord).rgb;

	m_Params.Metalness = texture(u_MetallicRoughnessAO, v_TexCoord).r;
	m_Params.Roughness = texture(u_MetallicRoughnessAO, v_TexCoord).g;
	float ao = texture(u_MetallicRoughnessAO, v_TexCoord).b;
	vec4 emission = texture(u_Emission, v_TexCoord);

	m_Params.View = normalize(v_CameraPosition - m_Params.WorldPos);
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
		float shadow = (1.0 - CalcDirectionalShadowFactor(light, NdotL, i));

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
