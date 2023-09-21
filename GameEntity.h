#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Transform.h"
#include "BufferStructs.h"
#include "DXCore.h"
#include <d3d11.h>
#include <memory>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh>);
	~GameEntity();

	// --= Methods =--

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform();

	// Draw method
	void DrawEntity(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
					Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer);

private:
	// --= Fields =--
	Transform transform;
	std::shared_ptr<Mesh> mesh;

	// Constants struct
	VertexShaderExternalData vsData;
};

