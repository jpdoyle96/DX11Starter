#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 position;	    // The local position of the vertex
	DirectX::XMFLOAT3 normal;		// The normal of the vertex
	DirectX::XMFLOAT2 uv;			// The uv coordinate of the vertex
	DirectX::XMFLOAT3 tangent;		// Tangent vector
};