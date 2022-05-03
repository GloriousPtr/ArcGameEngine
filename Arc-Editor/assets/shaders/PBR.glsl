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

	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

const float PI = 3.141592653589793;
const float EPSILON = 0.000000000000001;

uniform float u_IrradianceIntensity;
uniform float u_EnvironmentRotation;

/*
* u_Properties[0] = AlbedoColor: r, g, b, a;
* u_Properties[1] = Metallic, Roughness, unused, unused
* u_Properties[2] = EmissiveParams: r, g, b, intensity
* u_Properties[4] = UseAlbedMap, UseNormalMap, UseMRAMap, UseEmissiveMap
*/
uniform mat4 u_Properties;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_RadianceMap;
uniform sampler2D u_BRDFLutMap;

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
};

layout(location = 0) in VertexOutput Input;

// =========================================
layout(location = 0) out vec4 o_FragColor;
layout(location = 1) out vec4 o_Albedo;
layout(location = 2) out vec4 o_Position;
layout(location = 3) out vec4 o_Normal;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
};

PBRParameters m_Params;

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 GetNormalFromMap()
{
    vec3 tangentNormal = texture(u_NormalMap, Input.TexCoord).rgb * 2.0 - 1.0;
    return normalize(Input.WorldNormals * tangentNormal);
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

// =========================================
void main()
{
	vec4  albedo = u_Properties[0];
	float metalic = u_Properties[1].r;
	float roughness = u_Properties[1].g;
	vec4 emissiveParams = u_Properties[2];

	bool useAlbedoMap = u_Properties[3].x <= 0.5f ? false : true;
	bool useNormalMap = u_Properties[3].y <= 0.5f ? false : true;
	bool useMRAMap = u_Properties[3].z <= 0.5f ? false : true;
	bool useEmissiveMap = u_Properties[3].w <= 0.5f ? false : true;

    vec4 albedoWithAlpha = useAlbedoMap ? pow(texture(u_AlbedoMap, Input.TexCoord) * albedo, vec4(2.2, 2.2, 2.2, 1.0)) : albedo;
	if (albedoWithAlpha.a < 0.1)
		discard;

    m_Params.Albedo	= albedoWithAlpha.rgb;
    m_Params.Roughness = useMRAMap ? texture(u_MRAMap, Input.TexCoord).g * roughness : roughness;
	m_Params.Roughness = max(m_Params.Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight
    m_Params.Metalness = useMRAMap ? texture(u_MRAMap, Input.TexCoord).r * metalic : metalic;

    float ao = useMRAMap ? texture(u_MRAMap, Input.TexCoord).b : 1.0;

    vec3 emission = emissiveParams.w * (useEmissiveMap ? texture(u_EmissiveMap, Input.TexCoord).rgb : emissiveParams.rgb);

    m_Params.Normal = useNormalMap ? GetNormalFromMap() : normalize(Input.Normal);

    m_Params.View = normalize(Input.CameraPosition - Input.WorldPosition);
	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, m_Params.Albedo, m_Params.Metalness);
	
	vec3 ambient = IBL(F0) * ao;

    vec3 color = ambient + emission;

    o_FragColor = vec4(color, 1.0);
    o_Albedo = vec4(m_Params.Albedo, m_Params.Roughness);
	o_Position = vec4(Input.WorldPosition, 1.0);
	o_Normal = vec4(normalize(m_Params.Normal), 1.0);
}
