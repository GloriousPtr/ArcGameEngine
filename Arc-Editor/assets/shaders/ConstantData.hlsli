cbuffer Camera : register(b1, space0)
{
	row_major float4x4 ViewProjection;
}

SamplerState Sampler : register(s0);
