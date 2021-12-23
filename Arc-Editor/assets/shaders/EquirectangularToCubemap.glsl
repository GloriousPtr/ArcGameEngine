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

uniform sampler2D u_EquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(v_Position));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;

	o_Color = vec4(color, 1.0);
}
