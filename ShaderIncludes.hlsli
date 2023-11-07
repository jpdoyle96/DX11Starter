#ifndef __GGP__SHADER__INCLUDES__
#define __GGP__SHADER__INCLUDES__


// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;		// XYZ normal vector
	float2 uv				: TEXCOORD;		// uv texture coordinate
	float3 tangent			: TANGENT;		// tangent vector
};


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
	float3 tangent			: TANGENT;
	float3 worldPos			: POSITION;
};

// Special VertexToPixel for the SkyBox
struct VertexToPixel_SkyBox
{
	float4 position			: SV_POSITION;
	float3 sampleDir		: DIRECTION;
};


// Light types
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

#define MAX_SPECULAR_EXPONENT 256.0f

// Light struct
struct Light
{
	int		Type;
	float3	Direction;	

	float	Range;
	float3	Position;	

	float	Intensity;
	float3	Color;		

	float	SpotFalloff;
	float3	Padding;	
};


// Helper Functions

// Range-based attenuation function
float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);

	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));

	return att * att;
}

// Diffuse function
float Diffuse(float3 normal, float3 toLight)
{
	return saturate(dot(normal, toLight));
}

// Phong
float SpecularPhong(float3 normal, float3 toLight, float3 V, float roughness)
{
	if (roughness == 1.0f)
	{
		return 0.0f;
	}

	float3 R = reflect(-toLight, normal);

	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;

	return pow(max(dot(R, V), 0.0f), specExponent);
}



// Light Type Functions

float3 DirLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor, float specularScale)
{
	// Get light direction
	float3 toLight = normalize(-light.Direction);
	float3 V = normalize(camPos - worldPos);

	// Calculate the lighting value
	float diff = Diffuse(normal, toLight);
	float spec = SpecularPhong(normal, toLight, V, roughness) * specularScale;

	// Extreme angle cull
	spec *= any(diff);

	// Combine the results
	return (diff * surfaceColor + spec) * light.Color;
}

float3 PointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor, float specularScale)
{
	// Get light direction
	float3 toLight = normalize(light.Position - worldPos);
	float3 V = normalize(camPos - worldPos);

	// Calculate the lighting value
	float diff = Diffuse(normal, toLight);
	float spec = SpecularPhong(normal, toLight, V, roughness) * specularScale;
	float atten = Attenuate(light, worldPos);

	// Extreme angle cull
	spec *= any(diff);

	// Combine the results
	return (diff * surfaceColor + spec) * light.Color * atten;
}


// NOT IMPLEMENTED YET
float3 SpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor, float specularScale)
{
	// Get light direction
	float3 toLight = normalize(-light.Direction);

	// Calculate the lighting value
	float diff = Diffuse(normal, toLight);

	// Combine the results
	return (diff * surfaceColor * light.Color);
}



// Main light function
float3 CalcLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor, float specularScale)
{
	switch (light.Type)
	{
	case LIGHT_TYPE_DIRECTIONAL:
		return  DirLight(light, normal, worldPos, camPos, roughness, surfaceColor, specularScale);
	case LIGHT_TYPE_POINT:
		return  PointLight(light, normal, worldPos, camPos, roughness, surfaceColor, specularScale);
	case LIGHT_TYPE_SPOT:
		return  SpotLight(light, normal, worldPos, camPos, roughness, surfaceColor, specularScale);
	default:
		return float3(0.0f, 0.0f, 0.0f);
	}
}


#endif