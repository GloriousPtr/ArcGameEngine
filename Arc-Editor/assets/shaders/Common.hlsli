// Root constant
cbuffer Transform : register(b0, space0)
{
	float4x4 Model;
}

cbuffer Camera : register(b1, space0)
{
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
	float4 Position;
}

SamplerState Sampler : register(s0);
