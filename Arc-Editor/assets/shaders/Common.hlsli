// Root constant
cbuffer Transform : register(b0, space0)
{
	row_major float4x4 Model;
}

cbuffer Camera : register(b1, space0)
{
	row_major float4x4 View;
	row_major float4x4 Projection;
	row_major float4x4 ViewProjection;
	float4 Position;
}

SamplerState Sampler : register(s0);
