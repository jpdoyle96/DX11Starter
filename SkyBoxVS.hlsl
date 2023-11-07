#include "ShaderIncludes.hlsli"


cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}

// --------------------------------------------------------
// The entry point for the SkyBox Vertex shader
// --------------------------------------------------------
VertexToPixel_SkyBox main(VertexShaderInput input)
{
	// Output variable
	VertexToPixel_SkyBox output;

	// Copy and edit the view matrix
	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	// Multiply the viewNoTranslation and the projection
	matrix viewProj = mul(projection, viewNoTranslation);
	output.position = mul(viewProj, float4(input.localPosition, 1.0f));

	// Set the sky to be on the far clip plane at all times
	output.position.z = output.position.w;

	// Find sample direction based on input position
	output.sampleDir = input.localPosition;

	return output;
}