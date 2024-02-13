#include "Common.hlsli"

struct VertexIn
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD;
};

struct VertexOut
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;
	output.Position = v.Position;
	output.UV = v.UV;
	return output;
}

cbuffer Properties : register(b0, space1)
{
	uniform float4 Threshold; // x: threshold value (linear), y: knee, z: knee * 2, w: 0.25 / knee
	uniform float4 Params; // x: clamp, y: <1-unsampled, <2-downsample, <3-upsample, z: <1-prefilter, <=1-no prefilter, w: unused
	
	uint Texture;
	uint AdditiveTexture;
}

// Better, temporally stable box filtering
// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
float4 DownsampleBox13Tap(Texture2D tex, float2 uv, float2 texelSize)
{
	float4 A = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(-1.0, -1.0));
	float4 B = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(0.0, -1.0));
	float4 C = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(1.0, -1.0));
	float4 D = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(-0.5, -0.5));
	float4 E = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(0.5, -0.5));
	float4 F = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(-1.0, 0.0));
	float4 G = tex.Sample(LinearPointClampSampler, uv);
	float4 H = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(1.0, 0.0));
	float4 I = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(-0.5, 0.5));
	float4 J = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(0.5, 0.5));
	float4 K = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(-1.0, 1.0));
	float4 L = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(0.0, 1.0));
	float4 M = tex.Sample(LinearPointClampSampler, uv + texelSize * float2(1.0, 1.0));

	float2 div = (1.0 / 4.0) * float2(0.5, 0.125);

	float4 o = (D + E + I + J) * div.x;
	o += (A + B + G + F) * div.y;
	o += (B + C + H + G) * div.y;
	o += (F + G + L + K) * div.y;
	o += (G + H + M + L) * div.y;

	return o;
}

// Standard box filtering
float4 DownsampleBox4Tap(Texture2D tex, float2 uv, float2 texelSize)
{
	float4 d = texelSize.xyxy * float4(-1.0, -1.0, 1.0, 1.0);

	float4 s = tex.Sample(LinearPointClampSampler, uv + d.xy);
	s += tex.Sample(LinearPointClampSampler, uv + d.zy);
	s += tex.Sample(LinearPointClampSampler, uv + d.xw);
	s += tex.Sample(LinearPointClampSampler, uv + d.zw);

	return s * (1.0 / 4.0);
}

// 9-tap bilinear upsampler (tent filter)
float4 UpsampleTent(Texture2D tex, float2 uv, float2 texelSize, float4 sampleScale)
{
	float4 d = texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * sampleScale;

	float4 s = tex.Sample(LinearPointClampSampler, uv - d.xy);
	s += tex.Sample(LinearPointClampSampler, uv - d.wy) * 2.0;
	s += tex.Sample(LinearPointClampSampler, uv - d.zy);

	s += tex.Sample(LinearPointClampSampler, uv + d.zw) * 2.0;
	s += tex.Sample(LinearPointClampSampler, uv) * 4.0;
	s += tex.Sample(LinearPointClampSampler, uv + d.xw) * 2.0;

	s += tex.Sample(LinearPointClampSampler, uv + d.zy);
	s += tex.Sample(LinearPointClampSampler, uv + d.wy) * 2.0;
	s += tex.Sample(LinearPointClampSampler, uv + d.xy);

	return s * (1.0 / 16.0);
}

// Standard box filtering
float4 UpsampleBox(Texture2D tex, float2 uv, float2 texelSize, float4 sampleScale)
{
	float4 d = texelSize.xyxy * float4(-1.0, -1.0, 1.0, 1.0) * (sampleScale * 0.5);

	float4 s = tex.Sample(LinearPointClampSampler, uv + d.xy);
	s += tex.Sample(LinearPointClampSampler, uv + d.zy);
	s += tex.Sample(LinearPointClampSampler, uv + d.xw);
	s += tex.Sample(LinearPointClampSampler, uv + d.zw);

	return s * (1.0 / 4.0);
}

float4 PS_Main(VertexOut i) : SV_TARGET
{
	Texture2D tex = ResourceDescriptorHeap[Texture];
	
	float width = 0.0f;
	float height = 0.0f;
	tex.GetDimensions(width, height);
	float2 texelSize = float2(1.0, 1.0) / float2(width, height);
	float4 color = float4(0.0, 0.0, 0.0, 0.0);
	if (Params.y <= 1.01)
	{
		color = tex.Sample(LinearPointClampSampler, i.UV);
	}
	else if (Params.y <= 2.01)
	{
		color = DownsampleBox13Tap(tex, i.UV, texelSize);
	}
	else if (Params.y <= 3.01)
	{
		color = UpsampleTent(tex, i.UV, texelSize, float4(1.0, 1.0, 1.0, 1.0));
	}
	
	if (Params.z < 0.5)
	{
		// User controlled clamp to limit crazy high broken spec
		color = min(float4(Params.x, Params.x, Params.x, Params.x), color);

		float brightness = max(color.r, color.g);
		brightness = max(brightness, color.b);
		float softness = clamp(brightness - Threshold.x + Threshold.y, 0.0, Threshold.z);
		softness = (softness * softness) / (4.0 * Threshold.y + EPSILON);
		float multiplier = max(brightness - Threshold.x, softness) / max(brightness, EPSILON);
		color *= multiplier;
		color = max(color, float4(0.0, 0.0, 0.0, 1.0));
	}
	
	if (Params.w > 0.0)
	{
		Texture2D additiveTex = ResourceDescriptorHeap[AdditiveTexture];
		color += additiveTex.Sample(LinearPointClampSampler, i.UV);
	}
	
	return float4(color.rgb, 1.0);
}
