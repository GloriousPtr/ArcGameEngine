#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3 v_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_TexCoords = a_Position;
    gl_Position = u_Projection * mat4(mat3(u_View)) * vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

layout (location = 0) in vec3 v_TexCoords;

out vec4 fragColor;

uniform samplerCube u_EnvironmentMap;

const float PI = 3.14159265359;

void main()
{
	// vec3 envColor = texture(u_EnvironmentMap, v_TexCoords).rgb;
	
	// envColor = envColor / (envColor + vec3(1.0));
	// envColor = pow(envColor, vec3(1.0/2.2));

	vec3 normal = normalize(v_TexCoords);

	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up = cross(normal, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0;
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / float(nrSamples));

    fragColor = vec4(irradiance, 1.0);
}
