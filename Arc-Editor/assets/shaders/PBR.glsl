// Basic Texture Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec4 v_Color;
out vec2 v_TexCoord;

void main()
{
	v_Color = vec4(a_Normal, 1.0);
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	vec4 col = texture(u_Texture, v_TexCoord); // texture(u_Texture, v_TexCoord);
	color = col;
}
