#pragma once

#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <d3d11.h>
#include "DXCore.h"
#include "Vertex.h"
#include <DirectXMath.h>

class Mesh
{

public:
	Mesh(
		Vertex* vertices,
		int vertexCount,
		unsigned int* indices,
		int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	~Mesh();
	
	// Functions
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	void Draw();

	// Color tint
	DirectX::XMFLOAT4 GetColorTint();
	void SetColorTint(float r, float g, float b, float a);
	void SetColorTint(DirectX::XMFLOAT4 _colorTint);
	DirectX::XMFLOAT4& GetColorTintRef();

private:
	// Context
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context;

	// Counts
	int indexCount;

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Color tint
	DirectX::XMFLOAT4 colorTint;
};

