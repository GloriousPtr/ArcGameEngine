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

// x: ToneMapType
//		0: None/ExposureBased
//		1: ACES
//		2: Filmic
//		3: Uncharted
// y: Exposure
// z,w: unused
uniform vec4 u_TonemappParams;
uniform float u_BloomStrength;
uniform sampler2D u_Texture;
uniform sampler2D u_BloomTexture;

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_FragColor;

vec3 Uncharted2Tonemap(const vec3 x)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 TonemapUncharted2Exposure(const vec3 color)
{
	const float W = 11.2;
	const float exposureBias = 2.0;
	vec3 curr = Uncharted2Tonemap(exposureBias * color);
	vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
	return curr * whiteScale;
}

// Based on Filmic Tonemapping Operators http://filmicgames.com/archives/75
vec3 TonemapFilmic(const vec3 color)
{
	vec3 x = max(vec3(0.0), color - 0.004);
	return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 TonemapAces(const vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d ) + e), 0.0, 1.0);
}

vec3 ExposureTonemapping(const vec3 color)
{
	return vec3(1.0) - exp(-color);
}

void main()
{
	const float gamma = 2.2;
    vec3 hdrColor = texture(u_Texture, v_TexCoord).rgb;
	hdrColor += texture(u_BloomTexture, v_TexCoord).rgb * u_BloomStrength;
    vec3 result = vec3(1.0, 0.0, 1.0);

	uint type = uint(round(u_TonemappParams.x));
	// tone mapping
	switch (type)
	{
		case 0:
			result = ExposureTonemapping(u_TonemappParams.y * hdrColor);
			break;
		case 1:
			result = TonemapAces(u_TonemappParams.y * hdrColor);
			break;
		case 2:
			result = TonemapFilmic(u_TonemappParams.y * hdrColor);
			break;
		case 3:
			result = TonemapUncharted2Exposure(u_TonemappParams.y * hdrColor);
			break;
	}

    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / gamma));

    o_FragColor = vec4(result, 1.0);
}
