#include "ShaderIncludes.hlsli"


cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix worldInvTranspose;
	matrix view;
	matrix projection;

	matrix lightView;
	matrix lightProjection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
	matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Pass other data through for now
	output.uv = input.uv;
	output.normal = mul((float3x3)worldInvTranspose, input.normal);
	output.worldPos = mul(world, float4(input.localPosition, 1)).xyz;

	// Note: The tangent data will not be used by the basic PixelShader, but is 
	//		passed through regardless as it can be safely ignored in the PS
	output.tangent = mul((float3x3)world, input.tangent);

	// Shadow position calculations
	matrix shadowWVP = mul(lightProjection, mul(lightView, world));
	output.shadowMapPos = mul(shadowWVP, float4(input.localPosition, 1.0f));

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}