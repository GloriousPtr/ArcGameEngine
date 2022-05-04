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
};

layout(location = 0) out VertexOutput Output;

void main()
{
	Output.WorldPosition = vec3(u_Model * vec4(a_Position, 1.0));
    Output.TexCoord = a_TexCoord;
    Output.Normal = mat3(u_Model) * a_Normal;
	Output.WorldNormals = mat3(u_Model) * mat3(a_Tangent, a_Bitangent, a_Normal);
	Output.WorldTransform = mat3(u_Model);

	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

const float PI = 3.141592653589793;
const float EPSILON = 0.000000000000001;

/*
* u_Properties[0] = AlbedoColor: r, g, b, a;
* u_Properties[1] = Metallic, Roughness, unused, unused
* u_Properties[2] = EmissiveParams: r, g, b, intensity
* u_Properties[4] = UseAlbedMap, UseNormalMap, UseMRAMap, UseEmissiveMap
*/
uniform mat4 u_Properties;

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
};

layout(location = 0) in VertexOutput Input;

// =========================================
layout(location = 0) out vec4 o_Albedo;
layout(location = 1) out vec4 o_Position;
layout(location = 2) out vec4 o_Normal;
layout(location = 3) out vec4 o_MetallicRoughnessAO;
layout(location = 4) out vec4 o_Emission;

vec3 GetNormalFromMap()
{
    vec3 tangentNormal = texture(u_NormalMap, Input.TexCoord).rgb * 2.0 - 1.0;
    return normalize(Input.WorldNormals * tangentNormal);
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

    vec3 outAlbedo	= albedoWithAlpha.rgb;
    float outRoughness = useMRAMap ? texture(u_MRAMap, Input.TexCoord).g * roughness : roughness;
	outRoughness = max(outRoughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight
    float outMetalness = useMRAMap ? texture(u_MRAMap, Input.TexCoord).r * metalic : metalic;
    float outAO = useMRAMap ? texture(u_MRAMap, Input.TexCoord).b : 1.0;
    vec4 outEmission = useEmissiveMap ? texture(u_EmissiveMap, Input.TexCoord) : emissiveParams;
    vec3 outNormal = useNormalMap ? GetNormalFromMap() : normalize(Input.Normal);

    o_Albedo = vec4(outAlbedo, 1.0);
	o_Position = vec4(Input.WorldPosition, 1.0);
	o_Normal = vec4(normalize(outNormal), 1.0);
	o_MetallicRoughnessAO = vec4(outMetalness, outRoughness, outAO, 1.0);
	o_Emission = vec4(outEmission.rgb, outEmission.a / 255);
}
