
#define PI 3.14159265358979323846

// Buffer struct
cbuffer ExternalData : register(b0)
{
	float3 colorTint;
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
};

float sin_custom(in float coord, in float freq, in float hor_offset, in float vert_pos, in float scale)
{
	return scale * ((sin(2 * PI * freq * (coord - hor_offset)) + vert_pos) / 2);
}

float cos_custom(in float coord, in float freq, in float hor_offset, in float vert_pos, in float scale)
{
	return scale * ((cos(2 * PI * freq * (coord - hor_offset)) + vert_pos) / 2);
}

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
	float3 procedural = float3(cos_custom(input.uv.x, 88, 0.2, 2, 0.5) * sin_custom(input.uv.y, 33, 1, 1, 1), 
							cos_custom(input.uv.x, 22, 0.5, 2, 0.5) * sin_custom(input.uv.y, 77, 0.1, 1, 1),
							cos_custom(input.uv.x, 99, 0.8, 2, 0.5) * sin_custom(input.uv.y, 44, 0.6, 1, 1));
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	return float4(procedural * colorTint, 1);
}