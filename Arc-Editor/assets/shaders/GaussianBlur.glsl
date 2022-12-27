#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

uniform bool u_Horizontal;
uniform sampler2D u_Texture;

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_FragColor;

void main()
{
	vec4 color = vec4(1.0, 0.0, 1.0, 1.0);
	vec2 texelSize = 1.0 / textureSize(u_Texture, 0);

	if (u_Horizontal)
	{
		float texelSizeX = texelSize.x * 2;
		// 9-tap gaussian blur on the downsampled source
		vec4 c0 = texture(u_Texture, v_TexCoord - vec2(texelSizeX * 4.0, 0.0));
		vec4 c1 = texture(u_Texture, v_TexCoord - vec2(texelSizeX * 3.0, 0.0));
		vec4 c2 = texture(u_Texture, v_TexCoord - vec2(texelSizeX * 2.0, 0.0));
		vec4 c3 = texture(u_Texture, v_TexCoord - vec2(texelSizeX * 1.0, 0.0));
		vec4 c4 = texture(u_Texture, v_TexCoord                              );
		vec4 c5 = texture(u_Texture, v_TexCoord + vec2(texelSizeX * 1.0, 0.0));
		vec4 c6 = texture(u_Texture, v_TexCoord + vec2(texelSizeX * 2.0, 0.0));
		vec4 c7 = texture(u_Texture, v_TexCoord + vec2(texelSizeX * 3.0, 0.0));
		vec4 c8 = texture(u_Texture, v_TexCoord + vec2(texelSizeX * 4.0, 0.0));

		color = c0 * 0.01621622 + c1 * 0.05405405 + c2 * 0.12162162 + c3 * 0.19459459
					+ c4 * 0.22702703
					+ c5 * 0.19459459 + c6 * 0.12162162 + c7 * 0.05405405 + c8 * 0.01621622;
	}
	else
	{
		float texelSizeY = texelSize.y;
		// Optimized bilinear 5-tap gaussian on the same-sized source (9-tap equivalent)
        vec4 c0 = texture(u_Texture, v_TexCoord - vec2(0.0, texelSizeY * 3.23076923));
        vec4 c1 = texture(u_Texture, v_TexCoord - vec2(0.0, texelSizeY * 1.38461538));
        vec4 c2 = texture(u_Texture, v_TexCoord                                     );
        vec4 c3 = texture(u_Texture, v_TexCoord + vec2(0.0, texelSizeY * 1.38461538));
        vec4 c4 = texture(u_Texture, v_TexCoord + vec2(0.0, texelSizeY * 3.23076923));

        color = c0 * 0.07027027 + c1 * 0.31621622
                    + c2 * 0.22702703
                    + c3 * 0.31621622 + c4 * 0.07027027;
	}

	o_FragColor = vec4(color.rgb, 1.0);
}
