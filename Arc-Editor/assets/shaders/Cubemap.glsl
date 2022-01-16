#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

out vec3 v_Position;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	v_Position = a_Position;

	mat4 rotView = mat4(mat3(u_View));
	gl_Position = u_Projection * rotView * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

out vec4 o_Color;

in vec3 v_Position;

uniform samplerCube u_EnvironmentMap;

void main()
{
	vec3 color = texture(u_EnvironmentMap, v_Position).rgb;
	o_Color = vec4(color, 1.0);
}
