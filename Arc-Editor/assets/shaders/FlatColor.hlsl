cbuffer Textures : register(b0, space1)
{
	uint AlbedoTexture;
	uint NormalTexture;
};

cbuffer MaterialProperties : register(b1, space1)
{
	float3 AlbedoColor;
	float NormalStrength;
};

struct VertexIn
{
	float3 Position		: POSITION;
	float2 UV			: TEXCOORD;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float2 UV				: TEXCOORD;
	float3 Normal			: NORMAL;
};

struct Camera
{
	row_major float4x4 ViewProjection;
};

struct Properties
{
	row_major float4x4 Model;
};

ConstantBuffer<Camera> c_Camera : register(b1);
ConstantBuffer<Properties> c_Properties : register(b2);

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	float4 worldPosition = mul(float4(v.Position, 1.0), c_Properties.Model);
	output.Position = mul(worldPosition, c_Camera.ViewProjection);
	output.UV = v.UV;
	output.Normal = normalize(mul(c_Properties.Model, float4(v.Normal, 1.0)).xyz);

	return output;
}

SamplerState u_Sampler : register(s0);

float4 PS_Main(VertexOut input) : SV_TARGET
{
	Texture2D albedo = ResourceDescriptorHeap[AlbedoTexture];
	float4 color = albedo.Sample(u_Sampler, input.UV);
	return float4(color);
}
