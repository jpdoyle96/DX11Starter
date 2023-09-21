#include "Transform.h"

// For the DirectX Math library
using namespace DirectX;

// Constructor
Transform::Transform()
{
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());

	dirty = true;
	UpdateWorldMatrix();
}

// Destructor
Transform::~Transform()
{
}

// --= Setters =--

// Setter position floats
void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	dirty = true;
}

// Setter position vector
void Transform::SetPosition(DirectX::XMFLOAT3 _position)
{
	position = _position;
	dirty = true;
}

// Setter rotation floats
void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
	dirty = true;
}

// Setter rotation vector
void Transform::SetRotation(DirectX::XMFLOAT3 _rotation)
{
	rotation = _rotation;
	dirty = true;
}

// Setter scale floats
void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	dirty = true;
}

// Setter scale vector
void Transform::SetScale(DirectX::XMFLOAT3 _scale)
{
	scale = _scale;
	dirty = true;
}

// --= Getters =--

// Getter position
DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

// Getter rotation
DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}

// Getter scale
DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

// Getter world matrix
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateWorldMatrix();
	return worldMatrix;
}

// Getter world inverse transpose matrix
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateWorldMatrix();
	return worldInverseTransposeMatrix;
}

// --= Transformers =--

// Position transform floats
void Transform::MoveAbsolute(float x, float y, float z)
{
	position = XMFLOAT3(position.x + x, position.y + y, position.z + z);
	dirty = true;
}

// Position transform vector
void Transform::MoveAbsolute(DirectX::XMFLOAT3 _offset)
{
	position = XMFLOAT3(position.x + _offset.x, position.y + _offset.y, position.z + _offset.z);
	dirty = true;
}

// Rotation transform floats
void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(rotation.x + pitch, rotation.y + yaw, rotation.z + roll);
	dirty = true;
}

// Rotation transfrom vector
void Transform::Rotate(DirectX::XMFLOAT3 _rotation)
{
	rotation = XMFLOAT3(rotation.x + _rotation.x, rotation.y + rotation.y, rotation.z + rotation.z);
	dirty = true;
}

// Scale transform floats
void Transform::Scale(float x, float y, float z)
{
	scale = XMFLOAT3(scale.x * x, scale.y * y, scale.z * z);
	dirty = true;
}

// Scale transform vector 
void Transform::Scale(DirectX::XMFLOAT3 _scale)
{
	scale = XMFLOAT3(scale.x * _scale.x, scale.y * _scale.y, scale.z * _scale.z);
	dirty = true;
}

// --= Matrix methods =--

// Update world matrix
void Transform::UpdateWorldMatrix()
{
	// If the world matrix has been changed
	if (dirty == true)
	{
		// Calculate out the translation, rotation, and scaling matrices
		XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);
		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

		// Calculate the world matrix
		XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(scaleMatrix, rotationMatrix), translationMatrix);

		// Assign the world and world inverse transpose
		XMStoreFloat4x4(&worldMatrix, world);
		XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));

		// The matrix is now clean
		dirty = false;
	}
}
