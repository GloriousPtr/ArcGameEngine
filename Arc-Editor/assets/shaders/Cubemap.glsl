#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

out vec3 v_Position;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;

    vec3 u_CameraPosition;
};

void main()
{
	v_Position = a_Position;

	mat4 rotView = mat4(mat3(u_View));
	gl_Position = u_Projection * rotView * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Albedo;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_MetallicRoughnessAO;
layout(location = 3) out vec4 o_Emission;

in vec3 v_Position;

uniform samplerCube u_EnvironmentMap;
uniform float u_Intensity;
uniform float u_Rotation;

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
	angle = radians(angle);
	mat3x3 rotationMatrix = { vec3(cos(angle),0.0,sin(angle)),
							vec3(0.0,1.0,0.0),
							vec3(-sin(angle),0.0,cos(angle)) };
	return rotationMatrix * vec;
}

void main()
{
	vec3 color = textureLod(u_EnvironmentMap, RotateVectorAboutY(u_Rotation, v_Position), 0).rgb * u_Intensity;
	o_Albedo = vec4(color, 1.0);
	o_Normal = vec4(0.0);
	o_MetallicRoughnessAO = vec4(0.0);
	o_Emission = vec4(0.0);
}
