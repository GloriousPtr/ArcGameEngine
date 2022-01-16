// PBR

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;

    vec4 u_CameraPosition;
};

uniform mat4 u_Model;
uniform mat4 u_DirLightViewProj;

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;

	mat3 CameraView;
	vec3 CameraPosition;
	vec3 ViewPosition;

	vec4 DirLightViewProj;
};

layout(location = 0) out VertexOutput Output;

void main()
{
	Output.WorldPosition = vec3(u_Model * vec4(a_Position, 1.0));
    Output.TexCoord = a_TexCoord;
    Output.Normal = mat3(u_Model) * a_Normal;
	Output.WorldNormals = mat3(u_Model) * mat3(a_Tangent, a_Bitangent, a_Normal);
	Output.WorldTransform = mat3(u_Model);

	Output.CameraView = mat3(u_View);
    Output.CameraPosition = u_CameraPosition.xyz;
	Output.ViewPosition = vec3(u_View * vec4(Output.WorldPosition, 1.0));

	Output.DirLightViewProj = u_DirLightViewProj * u_Model * vec4(a_Position, 1.0);

	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

const float PI = 3.141592653589793;

const int MAX_NUM_LIGHTS = 25;

struct Light
{
    vec4 u_Position;
    vec4 u_Color;
    
    /* packed into a vec4
    x: radius
    y: falloff
    z: unused
    w: light type */
    vec4 u_AttenFactors;
    
    // Used for directional and spot lights
    vec4 u_LightDir;

    float u_Intensity;
};

layout (std140, binding = 1) uniform LightBuffer
{
    Light u_Lights[MAX_NUM_LIGHTS];
    uint u_NumLights;
};

uniform float u_IrradianceIntensity;

uniform vec4  u_Albedo;
uniform float u_NormalStrength;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AO;
uniform vec3  u_EmissionColor;
uniform float u_EmissiveIntensity;

uniform bool u_UseAlbedoMap;
uniform bool u_UseNormalMap;
uniform bool u_UseMRAMap;
uniform bool u_UseEmissiveMap;

uniform samplerCube u_IrradianceMap;
uniform sampler2D u_DirectionalShadowMap;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MRAMap;
uniform sampler2D u_EmissiveMap;

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;

	mat3 CameraView;
	vec3 CameraPosition;
	vec3 ViewPosition;

	vec4 DirLightViewProj;
};

layout(location = 0) in VertexOutput Input;

// =========================================
layout(location = 0) out vec4 o_FragColor;
layout(location = 1) out vec4 o_Position;
layout(location = 2) out vec4 o_Normal;

// N: Normal, H: Halfway, a2: pow(roughness, 4)
float DistributionGGX(const vec3 N, const vec3 H, const float a2)
{
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

// N: Normal, V: View, L: Light, k: roughness * roughness / 2.0
float GeometrySmith(const float NdotL, const float NdotV, const float k)
{
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	float oneMinusCosTheta = clamp(1.0 - cosTheta, 0.0, 1.0);
	float oneMinusCosTheta5 = oneMinusCosTheta * oneMinusCosTheta * oneMinusCosTheta * oneMinusCosTheta * oneMinusCosTheta;
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * oneMinusCosTheta5;
}

vec3 GetNormalFromMap()
{
    vec3 tangentNormal = texture(u_NormalMap, Input.TexCoord).rgb * 2.0 - 1.0;
    return normalize(Input.WorldNormals * tangentNormal);
}

float CalcDirectionalShadowFactor(Light directionalLight, const float NdotL)
{
	vec3 projCoords = Input.DirLightViewProj.xyz / Input.DirLightViewProj.w;
	projCoords = (projCoords * 0.5) + 0.5;

	float currentDepth = projCoords.z;

	float bias = max(0.0008 * (1.0 - NdotL), 0.0008);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_DirectionalShadowMap, 0);
	int sampleCount = 3;
	for(int x = -sampleCount; x <= sampleCount; ++x)
	{
		for(int y = -sampleCount; y <= sampleCount; ++y)
		{
			float pcfDepth = texture(u_DirectionalShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
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

// =========================================
void main()
{
    vec4 albedoWithAlpha	= u_UseAlbedoMap ? pow(texture(u_AlbedoMap, Input.TexCoord) * u_Albedo, vec4(2.2, 2.2, 2.2, 1.0)) : u_Albedo;
	if (albedoWithAlpha.a < 0.1)
		discard;

    vec3 albedo	= albedoWithAlpha.rgb;
    float metallic = u_UseMRAMap ? texture(u_MRAMap, Input.TexCoord).r : u_Metallic;
    float roughness = u_UseMRAMap ? texture(u_MRAMap, Input.TexCoord).g : u_Roughness;
    float ao = u_UseMRAMap ? texture(u_MRAMap, Input.TexCoord).b : u_AO;

    vec3 emission = u_EmissiveIntensity * (u_UseEmissiveMap ? texture(u_EmissiveMap, Input.TexCoord).rgb : u_EmissionColor);

    vec3 N = u_NormalStrength * (u_UseNormalMap ? GetNormalFromMap() : normalize(Input.Normal));

    vec3 V = normalize(Input.CameraPosition - Input.WorldPosition);
	float NdotV = max(dot(N, V), 0.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

	float a	= roughness * roughness;
	float a2 = a * a;
	float k = a * 0.5;

    // reflectance equation
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < u_NumLights; ++i)
    {
		Light light = u_Lights[i];
        uint type = uint(round(light.u_AttenFactors.w));
		bool isDirLight = type == 0;

        vec3 L;
		float NdotL;
		float shadow = 1.0;
        if (isDirLight)
        {
			L = -1.0 * normalize(light.u_LightDir.xyz);
			NdotL = max(dot(N, L), 0.0);
			shadow = (1.0 - CalcDirectionalShadowFactor(light, NdotL));
		}
		else
		{
			L = normalize(light.u_Position.rgb - Input.WorldPosition);
			NdotL = max(dot(N, L), 0.0);
		}

		if (shadow == 0.0)
			continue;

        vec3 H = normalize(L + V);

        float attenuation = 1.0;
        if (type == 1)
        {
            vec4 attenFactor = light.u_AttenFactors;
            float lightDistance2 = LengthSq(light.u_Position.xyz - Input.WorldPosition);
			float lightRadius2 = attenFactor.x * attenFactor.x;
			attenuation = clamp(1 - ((lightDistance2 * lightDistance2) / (lightRadius2 * lightRadius2)), 0.0, 1.0);
			attenuation = (attenuation * attenuation) / (lightDistance2 + 1.0);
            //attenuation = clamp(1.0 - (lightDistance2 / lightRadius2), 0.0, 1.0);
			//attenuation *= mix(attenuation, 1.0, attenFactor.y);
        }

        vec3 radiance = shadow * light.u_Color.rgb * light.u_Intensity * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, a2);
        float G = GeometrySmith(NdotL, NdotV, k);
        vec3 F = FresnelSchlickRoughness(max(dot(H, V), 0.0), F0, roughness);

        vec3 numerator = NDF * G * F;
        float denom = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular = numerator / denom;

		vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 kS = FresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(u_IrradianceMap, N).rgb * u_IrradianceIntensity;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (kD * diffuse) * ao;

    vec3 color = ambient + Lo + emission;

    o_FragColor = vec4(color, 1.0);
	o_Position = vec4(Input.WorldPosition, 1.0);
	o_Normal = vec4(Input.CameraView * normalize(Input.Normal), 1.0);
}
