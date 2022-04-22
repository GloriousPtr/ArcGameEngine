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

const float EPSILON = 1.0e-4;

uniform vec4 u_Threshold; // x: threshold value (linear), y: knee, z: knee * 2, w: 0.25 / knee
uniform vec4 u_Params; // x: clamp, y: <1-unsampled, <2-downsample, <3-upsample, z: <1-prefilter, <=1-no prefilter, w: unused
uniform sampler2D u_Texture;
uniform sampler2D u_AdditiveTexture;

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_FragColor;

// Better, temporally stable box filtering
// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
vec4 DownsampleBox13Tap(sampler2D tex, vec2 uv, vec2 texelSize)
{
    vec4 A = texture2D(tex, uv + texelSize * vec2(-1.0, -1.0));
    vec4 B = texture2D(tex, uv + texelSize * vec2( 0.0, -1.0));
    vec4 C = texture2D(tex, uv + texelSize * vec2( 1.0, -1.0));
    vec4 D = texture2D(tex, uv + texelSize * vec2(-0.5, -0.5));
    vec4 E = texture2D(tex, uv + texelSize * vec2( 0.5, -0.5));
    vec4 F = texture2D(tex, uv + texelSize * vec2(-1.0,  0.0));
    vec4 G = texture2D(tex, uv                               );
    vec4 H = texture2D(tex, uv + texelSize * vec2( 1.0,  0.0));
    vec4 I = texture2D(tex, uv + texelSize * vec2(-0.5,  0.5));
    vec4 J = texture2D(tex, uv + texelSize * vec2( 0.5,  0.5));
    vec4 K = texture2D(tex, uv + texelSize * vec2(-1.0,  1.0));
    vec4 L = texture2D(tex, uv + texelSize * vec2( 0.0,  1.0));
    vec4 M = texture2D(tex, uv + texelSize * vec2( 1.0,  1.0));

    vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

    vec4 o = (D + E + I + J) * div.x;
    o += (A + B + G + F) * div.y;
    o += (B + C + H + G) * div.y;
    o += (F + G + L + K) * div.y;
    o += (G + H + M + L) * div.y;

    return o;
}

// Standard box filtering
vec4 DownsampleBox4Tap(sampler2D tex, vec2 uv, vec2 texelSize)
{
    vec4 d = texelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0);

    vec4 s = texture2D(tex, uv + d.xy);
    s += texture2D(tex, uv + d.zy);
    s += texture2D(tex, uv + d.xw);
    s += texture2D(tex, uv + d.zw);

    return s * (1.0 / 4.0);
}

// 9-tap bilinear upsampler (tent filter)
vec4 UpsampleTent(sampler2D tex, vec2 uv, vec2 texelSize, vec4 sampleScale)
{
    vec4 d = texelSize.xyxy * vec4(1.0, 1.0, -1.0, 0.0) * sampleScale;

    vec4 s = texture2D(tex, uv - d.xy);
    s += texture2D(tex, uv - d.wy) * 2.0;
    s += texture2D(tex, uv - d.zy);

    s += texture2D(tex, uv + d.zw) * 2.0;
    s += texture2D(tex, uv       ) * 4.0;
    s += texture2D(tex, uv + d.xw) * 2.0;

    s += texture2D(tex, uv + d.zy);
    s += texture2D(tex, uv + d.wy) * 2.0;
    s += texture2D(tex, uv + d.xy);

    return s * (1.0 / 16.0);
}

// Standard box filtering
vec4 UpsampleBox(sampler2D tex, vec2 uv, vec2 texelSize, vec4 sampleScale)
{
    vec4 d = texelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0) * (sampleScale * 0.5);

    vec4 s = texture2D(tex, uv + d.xy);
    s += texture2D(tex, uv + d.zy);
    s += texture2D(tex, uv + d.xw);
    s += texture2D(tex, uv + d.zw);

    return s * (1.0 / 4.0);
}

void main()
{
	vec2 texelSize = 1.0 / textureSize(u_Texture, 0);
	vec4 color = vec4(0.0);
	if (u_Params.y <= 1.01)
	{
		color = texture2D(u_Texture, v_TexCoord);
	}
	else if (u_Params.y <= 2.01)
	{
		color = DownsampleBox13Tap(u_Texture, v_TexCoord, texelSize);
	}
	else if (u_Params.y <= 3.01)
	{
		color = UpsampleTent(u_Texture, v_TexCoord, texelSize, vec4(1.0));
	}
	
	if (u_Params.z < 0.5)
	{
		// User controlled clamp to limit crazy high broken spec
		color = min(vec4(u_Params.x), color);

		float brightness = max(color.r, color.g);
		brightness = max(brightness, color.b);
		float softness = clamp(brightness - u_Threshold.x + u_Threshold.y, 0.0, u_Threshold.z);
		softness = (softness * softness) / (4.0 * u_Threshold.y + EPSILON);
		float multiplier = max(brightness - u_Threshold.x, softness) / max(brightness, EPSILON);
		color *= multiplier;
		color = max(color, vec4(0.0, 0.0, 0.0, 1.0));
	}
	
	if (u_Params.w > 0.0)
	{
		color += texture2D(u_AdditiveTexture, v_TexCoord);
	}
	
	o_FragColor = vec4(color.rgb, 1.0);
}
