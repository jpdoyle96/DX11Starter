#pragma once
# include <DirectXMath.h>

#include "Transform.h"

class Camera
{
public:
	Camera(
		DirectX::XMFLOAT3 position,
		float moveSpeed,
		float mouseLookSpeed,
		float fieldOfView,
		float aspectRatio,
		float nearClip = 0.01f,
		float farClip = 100.0f);

	Camera(
		float x,
		float y,
		float z,
		float moveSpeed,
		float mouseLookSpeed,
		float fieldOfView,
		float aspectRatio,
		float nearClip = 0.01f,
		float farClip = 100.0f);

	~Camera();

	// Getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	Transform& GetTransform();
	
	float GetFieldOfView();
	float GetMovementSpeed();
	float GetMouseLookSpeed();
	float GetNearClip();
	float GetFarClip();
	
	// Setters
	void SetFieldOfView(float fov);
	void SetMovementSpeed(float speed);
	void SetMouseLookSpeed(float speed);
	void SetNearClip(float distance);
	void SetFarClip(float distance);
	
	// Updaters
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float _aspectRatio);

private:
	// Matrices
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	Transform transform;

	float movementSpeed;
	float mouseLookSpeed;
	float fieldOfView;
	float aspectRatio;
	float nearClip;
	float farClip;
};

