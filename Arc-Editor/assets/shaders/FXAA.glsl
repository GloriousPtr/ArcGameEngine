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

#define FXAA_SPAN_MAX 8.0

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_FragColor;

// Current Threshold
// Trims the algorithm from processing darks.
//		0.0833 - upper limit (default, the start of visible unfiltered edges)
//		0.0625 - low quality (faster)
//		0.0312 - high quality (slower)
//		0.0078125 - ultra quality (slower)

// Relative Threshold
// The minimum amount of local contrast required to apply algorithm.
//		0.333 - too little (faster)
//		0.250 - low quality
//		0.166 - medium quality
//		0.125 - high quality 
//		0.063 - ultra quality (slower)

uniform vec2 u_Threshold;				/* x: current threshold, y: relative threshold */
uniform sampler2D u_Texture;

void main()
{
	vec2 textureSize = textureSize(u_Texture, 0);
	vec2 texelSize = vec2(1.0 / textureSize.x, 1.0 / textureSize.y);

	// Samples the texels around and calculate their corresponding luminosity
	vec3 calculateLuma = vec3(0.299, 0.587, 0.114);
	vec3 rgbM  = texture(u_Texture, v_TexCoord).xyz;
	vec3 rgbNW = texture(u_Texture, v_TexCoord + (vec2(-1.0,-1.0)) * texelSize).xyz;
	vec3 rgbNE = texture(u_Texture, v_TexCoord + (vec2(1.0,-1.0)) * texelSize).xyz;
	vec3 rgbSW = texture(u_Texture, v_TexCoord + (vec2(-1.0,1.0)) * texelSize).xyz;
	vec3 rgbSE = texture(u_Texture, v_TexCoord + (vec2(1.0,1.0)) * texelSize).xyz;

	float lumaM  = dot(rgbM,  calculateLuma);
	float lumaNW = dot(rgbNW, calculateLuma);
	float lumaNE = dot(rgbNE, calculateLuma);
	float lumaSW = dot(rgbSW, calculateLuma);
	float lumaSE = dot(rgbSE, calculateLuma);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); 

	// Calculate sample direction
	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * u_Threshold.y), u_Threshold.x);
	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * texelSize;

	// Perform the samples and calculate the new texel colour
	vec3 rgbA = 0.5 * (texture(u_Texture, v_TexCoord + dir * ((1.0 / 3.0) - 0.5)).xyz + texture(u_Texture, v_TexCoord + dir * ((2.0 / 3.0) - 0.5)).xyz);
	vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(u_Texture, v_TexCoord + dir * - 0.5).xyz + texture(u_Texture, v_TexCoord + dir * 0.5).xyz);
	float lumaB = dot(rgbB, calculateLuma);

	if ((lumaB < lumaMin) || (lumaB > lumaMax))
		o_FragColor = vec4(rgbA, 1.0);
	else
		o_FragColor = vec4(rgbB, 1.0);
}
