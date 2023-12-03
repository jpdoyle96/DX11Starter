#include "ShaderIncludes.hlsli"

// Buffer struct
cbuffer ExternalData : register(b0)
{
	float roughness;
	float3 colorTint;
	float3 ambientColor;
	float3 cameraPosition;
	Light lights[6];
}


// Texture related resources
Texture2D Albedo			: register(t0); // Textures use "t" registers
Texture2D RoughnessMap		: register(t1);
Texture2D MetalnessMap		: register(t2);
Texture2D NormalMap			: register(t3);
Texture2D ShadowMap			: register(t4);

SamplerState BasicSampler				: register(s0); // Samplers use "s" registers
SamplerComparisonState ShadowSampler	: register(s1);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Shadow mapping
	// Perform the perspective divide (divide by W) ourselves
	input.shadowMapPos /= input.shadowMapPos.w;

	// Convert the normalized device coordinates to UVs for sampling
	float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y; // Flip the Y

	// Grab the distances we need: light-to-pixel and closest-surface
	float distToLight = input.shadowMapPos.z;

	// Get a ratio of comparison results using SampleCmpLevelZero()
	float shadowAmount = ShadowMap.SampleCmpLevelZero(
		ShadowSampler,
		shadowUV,
		distToLight).r;

	// Normal mapping
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	unpackedNormal = normalize(unpackedNormal);

	// Create TBN matrix
	float3 N = normalize(input.normal);
	float3 T = normalize(input.tangent);
	T = normalize(T - N * dot(T, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Remap normal
	input.normal = mul(unpackedNormal, TBN);

	float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
	surfaceColor *= colorTint;

	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;

	float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);

	float3 total = surfaceColor * ambientColor;

	for (int i = 0; i < 6; i++)
	{
		Light light = lights[i];
		light.Direction = normalize(light.Direction);

		float3 lightResult = CalcLight(light, input.normal, input.worldPos, cameraPosition, roughness, metalness, surfaceColor, specularColor);

		if (i == 5)	// My directional shadow emitting light
		{
			lightResult *= shadowAmount;
		}

		total += lightResult;
	}

	return float4(pow(total, 1.0f / 2.2f), 1);
}