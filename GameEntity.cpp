#include "GameEntity.h"

// Constructor
GameEntity::GameEntity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material) :
	mesh(_mesh),
	material(_material)
{
    transform = Transform();
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

// Material getter
std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}

// Material setter
void GameEntity::SetMaterial(std::shared_ptr<Material> _material)
{
	material = _material;
}


// Draw method
void GameEntity::DrawEntity(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	// Update constant buffer
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	ps->SetFloat3("colorTint", material->GetColorTint());
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	
	// Map the data
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	// Activate the shaders
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	// Draw the mesh
	mesh->Draw();
}

