struct MaterialData
{
	uint AlbedoTexture;
};

ConstantBuffer<MaterialData> Material : register(b0);

struct VertexIn
{
	float3 Position		: POSITION;
	float2 UV			: TEXCOORD;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float2 UV				: TEXCOORD;
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

	return output;
}

SamplerState u_Sampler : register(s0);

float4 PS_Main(VertexOut input) : SV_TARGET
{
	Texture2D albedo = ResourceDescriptorHeap[Material.AlbedoTexture];
	float4 color = albedo.Sample(u_Sampler, input.UV);
	return float4(color);
}
