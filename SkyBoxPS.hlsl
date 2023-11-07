#include "ShaderIncludes.hlsli"


// Texture resources
TextureCube SkyTexture		: register(t0);
SamplerState BasicSampler	: register(s0);

// --------------------------------------------------------
// The entry point for the SkyBox Pixel shader
// --------------------------------------------------------
float4 main(VertexToPixel_SkyBox input) : SV_TARGET
{
	// Samples the cube map and return
	//return SkyTexture.Sample(BasicSampler, input.sampleDir);
	return SkyTexture.Sample(BasicSampler, input.sampleDir);
}