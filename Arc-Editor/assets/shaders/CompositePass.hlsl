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
	float4 VignetteColor; // rgb: color, a: intensity
	float4 VignetteOffset; // xy: offset, z: useMask, w: enable/disable effect
	float TonemapExposure;
	uint TonemapType; // 0 None/ExposureBased, 1: ACES, 2: Filmic, 3: Uncharted
	
	uint MainTexture;
	uint VignetteMask;
}

float3 Uncharted2Tonemap(const float3 x)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 TonemapUncharted2Exposure(const float3 color)
{
	const float W = 11.2;
	const float exposureBias = 2.0;
	float3 curr = Uncharted2Tonemap(exposureBias * color);
	float3 whiteScale = 1.0 / Uncharted2Tonemap(float3(W, W, W));
	return curr * whiteScale;
}

// Based on Filmic Tonemapping Operators http://filmicgames.com/archives/75
float3 TonemapFilmic(const float3 color)
{
	float3 x = max(float3(0.0, 0.0, 0.0), color - 0.004);
	return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 TonemapAces(const float3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0));
}

float3 ExposureTonemapping(const float3 color)
{
	return float3(1.0, 1.0, 1.0) - exp(-color);
}

float4 PS_Main(VertexOut i) : SV_TARGET
{
	const float invGamma = 1.0 / 2.2;
	
	Texture2D mainTex = ResourceDescriptorHeap[MainTexture];
	float3 color = mainTex.Sample(Sampler, i.UV).rgb;
	
	// tone mapping
	float3 result = float3(1.0, 0.0, 1.0);
	switch (TonemapType)
	{
		case 0:
			result = ExposureTonemapping(TonemapExposure * color);
			break;
		case 1:
			result = TonemapAces(TonemapExposure * color);
			break;
		case 2:
			result = TonemapFilmic(TonemapExposure * color);
			break;
		case 3:
			result = TonemapUncharted2Exposure(TonemapExposure * color);
			break;
	}

	// Apply vignette
	if (VignetteOffset.w > 0.0)
	{
		if (VignetteOffset.z > 0.0)
		{
			Texture2D vignetteTex = ResourceDescriptorHeap[VignetteMask];
			float vignetteOpacity = (1.0 - vignetteTex.Sample(Sampler, i.UV).r) * VignetteColor.a;
			result = lerp(result, VignetteColor.rgb, vignetteOpacity);
		}
		else
		{
			float2 uv = i.UV + VignetteOffset.xy;
			uv *= 1.0 - (i.UV.yx + VignetteOffset.yx);
			float vig = uv.x * uv.y * 15.0;
			vig = pow(vig, VignetteColor.a);
			vig = clamp(vig, 0.0, 1.0);

			result = lerp(VignetteColor.rgb, result, vig);
		}
	}

    // also gamma correct while we're at it
	result = pow(result, float3(invGamma, invGamma, invGamma));

	return float4(result, 1.0);
}
