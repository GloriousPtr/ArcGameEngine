#include "MathsConstants.hlsli"

TextureCube<float4> cubeMapTexture : register(t0);
RWTexture2DArray<float4> outputIrradianceMap : register(u0);

SamplerState Sampler : register(s0);
SamplerState LinearClampSampler : register(s1);

static const uint SAMPLES = 16384;
static const float INV_SAMPLES = 1.0f / (float) SAMPLES;

// Reference : https://learnopengl.com/PBR/IBL/Diffuse-irradiance


// This shader is used to compute the diffuse part of the cook torrence BRDF by considering each cube map texel
// to be a light emitter. The diffuse part of the BRDF is only dependent on wi, the incoming light direction.
// We want to precompute the diffuse irradiance map so that we can sample a texture, and receive the irradiance along 
// the view direction w0. In this case, the view direction wo in the PBR shader will be the objects normal.
// The integral to be computed is [integral over hemisphere at point p] (Li(wi, p) * n.wi)dwi. Li(wi, p) is the radiance along the direction
// wi, which can be obtained by just sampling the cube map, and n.wi is the dot product between all such incoming light directions wi and the sampling vector n
// over the hemisphere. Since this integral cannot be solved analytically, we use a quasi-montecarlo integration technique to compute it.
// Using a low discrepancy sequence (such as the Hammerley sequence), we get uniformly distributed sample points in range [0, 1].
// Passing this into a function that returns samples from a hemisphere where z is between 0 and 1.
// Since this hemisphere sampled vector is in its own coordinate form, similar to normal mapping we compute a change of basis matrix (or in this case just 
// transform the coordinate) from its own local space to 'world space' (i.e the space where the sampling vector forms one of the orthonormal basis vectors).
// We can then compute n.wi, and as per montecarlo integration:
// Integral f(x) dx = 1 / N * summation f(xi)/pdf(xi) where pdf is the probability density function. 
// For a hemisphere, PDF is 1 / (2 PI). The lambert diffuse term is C / PI, Our result from this shader needs to be multiplied with PI * 2.0 and divide by number of samples.
// We repeat this process for several number of samples.

float vanDerCorputRadicalInverse(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	return float(bits) * 2.3283064365386963e-10;
}

// Get the i'th point from Hammersley sequence, a low discrepancy sequence.
float2 sampleHammersleySequence(uint i)
{
	return float2((float) i * INV_SAMPLES, vanDerCorputRadicalInverse(i));
}

// Source used :
// https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations Two
// uniform random numbers are provided in uv, and a vector on the hemisphere is returned.
float3 uniformSampleHemisphere(float2 uv)
{
	const float z = uv.x;
	const float r = pow(max(0.0f, 1.0f - z * z), 0.5f);
	const float phi = 2.0f * PI * uv.y;
	return float3(r * cos(phi), r * sin(phi), z);
}

// Bring the vector that was randomly sampled from a hemisphere into the coordinate system where N, S and T form the orthonormal basis.
float3 tangentToWorldCoords(float3 v, float3 n, float3 s, float3 t)
{
	return s * v.x + t * v.y + n * v.z;
}

float3 getSamplingVector(float2 pixelCoords, uint3 dispatchThreadID)
{
    // Convert pixelCoords into the range of -1 .. 1 and make sure y goes from top to bottom.
	float2 uv = 2.0f * float2(pixelCoords.x, 1.0f - pixelCoords.y) - float2(1.0f, 1.0f);

	float3 samplingVector = float3(0.0f, 0.0f, 0.0f);

    // Based on cube face 'index', choose a vector.
	switch (dispatchThreadID.z)
	{
		case 0:
			samplingVector = float3(1.0, uv.y, -uv.x);
			break;
		case 1:
			samplingVector = float3(-1.0, uv.y, uv.x);
			break;
		case 2:
			samplingVector = float3(uv.x, 1.0, -uv.y);
			break;
		case 3:
			samplingVector = float3(uv.x, -1.0, uv.y);
			break;
		case 4:
			samplingVector = float3(uv.x, uv.y, 1.0);
			break;
		case 5:
			samplingVector = float3(-uv.x, uv.y, -1.0);
			break;
	}

	return normalize(samplingVector);
}

// Given 3 vectors, compute a orthonormal basis using them.
// Will be used for diffuse irradiance cube map calculation to create a set of basis vectors
// to convert from the shading / tangent space to world space.
// Reference :
// https://github.com/Nadrin/PBR/blob/cd61a5d59baa15413c7b0aff4a7da5ed9cc57f61/data/shaders/hlsl/irmap.hlsl#L73
void computeBasisVectors(float3 normal, out float3 s, out float3 t)
{
	t = cross(normal, float3(0.0f, 1.0f, 0.0f));
	t = lerp(cross(normal, float3(1.0f, 0.0f, 0.0f)), t, step(MIN_FLOAT_VALUE, dot(t, t)));

	t = normalize(t);
	s = normalize(cross(normal, t));
}

// Calculates the variable diffuse parts of the diffuse brdf (integral n.wi * li(p, wi)).
[numthreads(8, 8, 1)]
void CS_Main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float textureWidth, textureHeight, textureDepth;
	outputIrradianceMap.GetDimensions(textureWidth, textureHeight, textureDepth);

    // For each of the 6 cube faces, there will be threads which have the same 2d pixel coord. 
	const float2 pixelCoords = (dispatchThreadID.xy + 0.5f) / float2(textureWidth, textureHeight);

    // Based on current texture pixel coord (and the dispatch's z parameter : or the number of groupz on the z axis
    // (hardcoded to 6), it will calculate the normalized samling direction which we use to sample into the cube map.
    // Reference : https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/irmap.hlsl
	const float3 samplingVector = getSamplingVector(pixelCoords, dispatchThreadID);

    // Calculation of basis vectors for converting a vector from Shading / Tangent space to world space.
	const float3 normal = normalize(samplingVector);
	float3 t = float3(0.0f, 0.0f, 0.0f);
	float3 s = float3(0.0f, 0.0f, 0.0f);

	computeBasisVectors(normal, s, t);

    // Using Monte Carlo integration to find irradiance of the hemisphere.
    // The final result should be 1 / N * summation (f(x) / PDF). PDF for sampling uniformly from hemisphere is 1 /
    // TWO_PI. 
	float3 irradiance = float3(0.0f, 0.0f, 0.0f);

    // This loop, will for each sampling direction (i.e the surface normal) will generate random points on the
    // hemisphere at p, get its world space coordinate such that it is centered at the particular pixel / point, and
    // convolute the input cube map to average the radiance, so as to get the resultant irradiance from a particular wi
    // direction.
	for (uint i = 0; i < SAMPLES; i++)
	{
        // Get a uniform random variable which will be used to get a point on the hemisphere.

		const float2 uv = sampleHammersleySequence(i);

		const float3 li = tangentToWorldCoords(uniformSampleHemisphere(uv), normal, s, t);

		const float cosTheta = saturate(dot(li, normal));

		irradiance += cubeMapTexture.SampleLevel(LinearClampSampler, li, 0u).rgb * cosTheta;
	}

    // PI cancels out as lambertian diffuse divides by PI.
	outputIrradianceMap[dispatchThreadID] = float4(irradiance * 2.0f * INV_SAMPLES, 1.0f);
}
