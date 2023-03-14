static const float PI = 3.141592653589793;
static const float EPSILON = 1.17549435E-38;

struct VertexIn
{
	float3 Position		: POSITION;
	float4 Color		: COLOR;
};

struct VertexOut
{
	float4 Position			: SV_POSITION;
	float4 Color			: COLOR;
};

struct Camera
{
	row_major float4x4 ViewProjection;
};

struct Properties
{
	row_major float4x4 Model;
};

ConstantBuffer<Camera> c_Camera : register(b0);
ConstantBuffer<Properties> c_Properties : register(b1);

VertexOut VS_Main(VertexIn v)
{
	VertexOut output;

	float4 worldPosition = mul(float4(v.Position, 1.0), c_Properties.Model);
	output.Position = mul(worldPosition, c_Camera.ViewProjection);
	output.Color = v.Color;

	return output;
}

struct PointLight
{
	float4 Position;			// xyz: position, w: radius
	float4 Color;				// rgb: color, a: intensity
};

struct DirectionalLight
{
	float4 Direction;
	float4 Color;				// rgb: color, a: intensity
};

StructuredBuffer<DirectionalLight> u_DirectionalLights : register (t0);
StructuredBuffer<PointLight> u_PointLights : register (t1);

Texture2D u_Albedo : register(t2);
Texture2D u_NormalMap : register(t3);

SamplerState u_Sampler : register(s0);

struct MaterialData
{
	float4 MRAO;
};

ConstantBuffer<MaterialData> c_Material : register(b2);

float4 PS_Main(VertexOut input) : SV_TARGET
{
	return float4(input.Color);
}
