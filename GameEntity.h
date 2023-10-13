#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "DXCore.h"
#include "Camera.h"
#include <d3d11.h>
#include <memory>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~GameEntity();

	// --= Methods =--

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform();
	std::shared_ptr<Material> GetMaterial();

	// Setters
	void SetMaterial(std::shared_ptr<Material> material);

	// Draw method
	void DrawEntity(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
					std::shared_ptr<Camera> camera);

private:
	// --= Fields =--
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

