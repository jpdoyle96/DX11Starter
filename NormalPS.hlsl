#include "ShaderIncludes.hlsli"

// Buffer struct
cbuffer ExternalData : register(b0)
{
	float roughness;
	float3 colorTint;
	float3 ambientColor;
	float3 cameraPosition;
	Light lights[5];
}


// Texture related resources
Texture2D SurfaceTexture	: register(t0); // Textures use "t" registers
Texture2D Specular			: register(t1);
Texture2D NormalMap			: register(t2);

SamplerState BasicSampler	: register(s0); // Samplers use "s" registers

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

	float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
	surfaceColor *= colorTint;

	float specular = 1.0f - Specular.Sample(BasicSampler, input.uv).r;

	float3 total = surfaceColor * ambientColor;

	for (int i = 0; i < 5; i++)
	{
		Light light = lights[i];
		light.Direction = normalize(light.Direction);

		total += CalcLight(light, input.normal, input.worldPos, cameraPosition, roughness, surfaceColor, specular);
	}

	return float4(total, 1);
}