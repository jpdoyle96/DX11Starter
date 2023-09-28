#include "Camera.h"
#include "Input.h"

using namespace DirectX;

// Constructor
Camera::Camera(DirectX::XMFLOAT3 position, float moveSpeed, float mouseLookSpeed, float fieldOfView, float aspectRatio, float nearClip, float farClip) :
		movementSpeed(moveSpeed),
		mouseLookSpeed(mouseLookSpeed),
		fieldOfView(fieldOfView),
		aspectRatio(aspectRatio),
		nearClip(nearClip),
		farClip(farClip)
{
	transform.SetPosition(position);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

// Constructor
Camera::Camera(float x, float y, float z, float moveSpeed, float mouseLookSpeed, float fieldOfView, float aspectRatio, float nearClip, float farClip) :
		movementSpeed(moveSpeed),
		mouseLookSpeed(mouseLookSpeed),
		fieldOfView(fieldOfView),
		aspectRatio(aspectRatio),
		nearClip(nearClip),
		farClip(farClip)
{
	transform.SetPosition(x, y, z);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

// Destructor
Camera::~Camera()
{ }


// --= Getters =--

// Get view matrix
DirectX::XMFLOAT4X4 Camera::GetView()
{
	return view;
}

// Get projection matrix
DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projection;
}

// Get transform
Transform& Camera::GetTransform()
{
	return transform;
}

// Get field of view
float Camera::GetFieldOfView()
{
	return fieldOfView;
}

// Get movement speed
float Camera::GetMovementSpeed()
{
	return movementSpeed;
}

// Get mouse look speed
float Camera::GetMouseLookSpeed()
{
	return mouseLookSpeed;
}

// Get near clip plane distance
float Camera::GetNearClip()
{
	return nearClip;
}

// Get far clip plane distance
float Camera::GetFarClip()
{
	return farClip;
}


// --= Setters =--

// Set field of view
void Camera::SetFieldOfView(float fov)
{
	fieldOfView = fov;
	UpdateProjectionMatrix(aspectRatio);
}

// Set movement speed
void Camera::SetMovementSpeed(float speed)
{
	movementSpeed = speed;
}

// Set mouse look speed
void Camera::SetMouseLookSpeed(float speed)
{
	mouseLookSpeed = speed;
}

// Set near clip plane distance
void Camera::SetNearClip(float distance)
{
	nearClip = distance;
	UpdateProjectionMatrix(aspectRatio);
}

// Set far clip plane distance
void Camera::SetFarClip(float distance)
{
	farClip = distance;
	UpdateProjectionMatrix(aspectRatio);
}


// --= Updaters =--

// Update camera
void Camera::Update(float dt)
{
	// Get the input manager
	Input& input = Input::GetInstance();

	// Get current speed
	float speed = dt * movementSpeed;

	// Increase or decrease speed
	if (input.KeyDown(VK_SHIFT)) { speed *= 5; }
	if (input.KeyDown(VK_CONTROL)) { speed != 0.1f; }

	// Movement
	if (input.KeyDown('W')) { transform.MoveRelative(0.0f, 0.0f, speed); }
	if (input.KeyDown('A')) { transform.MoveRelative(-speed, 0.0f, 0.0f); }
	if (input.KeyDown('S')) { transform.MoveRelative(0.0f, 0.0f, -speed); }
	if (input.KeyDown('D')) { transform.MoveRelative(speed, 0.0f, 0.0f); }
	if (input.KeyDown('X')) { transform.MoveRelative(0.0f, -speed, 0.0f); }
	if (input.KeyDown(' ')) { transform.MoveRelative(0.0f, speed, 0.0f); }

	// Handle mouse movement only when left clicking
	if (input.MouseLeftDown())
	{
		// Calculate cursor position change
		float xDist = mouseLookSpeed * input.GetMouseXDelta();
		float yDist = mouseLookSpeed * input.GetMouseYDelta();
		transform.Rotate(yDist, xDist, 0.0f);

		// Clamp the X rotation
		XMFLOAT3 rot = transform.GetPitchYawRoll();
		if (rot.x > XM_PIDIV2) rot.x = XM_PIDIV2;
		if (rot.x < -XM_PIDIV2) rot.x = -XM_PIDIV2;
		transform.SetRotation(rot);
	}

	// Update the view matrix
	UpdateViewMatrix();
}

// Update view matrix
void Camera::UpdateViewMatrix()
{
	// Get the forward vector and position of the camera
	XMFLOAT3 forward = transform.GetForward();
	XMFLOAT3 position = transform.GetPosition();

	// Create the view matrix
	XMMATRIX viewMatrix = XMMatrixLookToLH(
		XMLoadFloat3(&position),
		XMLoadFloat3(&forward),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&view, viewMatrix);
}

// Update projection matrix
void Camera::UpdateProjectionMatrix(float _aspectRatio)
{
	aspectRatio = _aspectRatio;

	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(
		fieldOfView,
		aspectRatio,
		nearClip,
		farClip);
	XMStoreFloat4x4(&projection, projMatrix);
}
