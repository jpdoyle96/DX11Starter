#include "GameEntity.h"

// Constructor
GameEntity::GameEntity(std::shared_ptr<Mesh> _mesh)
{
    mesh = _mesh;
    transform = Transform();

    // Default vertex shader values
    vsData.colorTint = mesh->GetColorTint();
    vsData.worldMatrix = transform.GetWorldMatrix();
}

// Destructor
GameEntity::~GameEntity()
{
}

// Mesh getter
std::shared_ptr<Mesh> GameEntity::GetMesh()
{
    return mesh;
}

// Transform getter
Transform& GameEntity::GetTransform()
{
    return transform;
}

// Draw method
void GameEntity::DrawEntity(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer, std::shared_ptr<Camera> camera)
{
	// Gather updated vsData
	vsData.colorTint = mesh->GetColorTint();
	vsData.worldMatrix = transform.GetWorldMatrix();
	vsData.viewMatrix = camera->GetView();
	vsData.projectionMatrix = camera->GetProjection();
	

	// Constant buffer data
	{
		// Mapping the resource
		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
		context->Unmap(vsConstantBuffer.Get(), 0);

		// Bind resource
		context->VSSetConstantBuffers(
			0,	// Slot (register) to bind to
			1,	// How many to activate
			vsConstantBuffer.GetAddressOf());	// Array of buffers
	}

	// Draw the mesh
	mesh->Draw();
}
