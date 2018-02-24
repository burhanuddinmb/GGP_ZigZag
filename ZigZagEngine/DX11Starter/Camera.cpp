#include "Camera.h"

Camera::Camera(int width, int height)
{
	direction = XMFLOAT3(0, 0, 1);
	position = XMFLOAT3(0, 0, -5);

	this->width = width;
	this->height = height;
	MakeViewMatrix();
	MakeProjectionMatrix();
}

void Camera::MakeViewMatrix()
{
	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR positionVector = XMLoadFloat3(&position);
	XMVECTOR directionVector = XMLoadFloat3(&direction);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);

	XMMATRIX V = XMMatrixLookToLH(
		positionVector,     // The position of the "camera"
		directionVector,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!	
	
}

void Camera::MakeProjectionMatrix()
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

void Camera::OnResize(int width, int height)
{
	this->width = width;
	this->height = height;
	// Update our projection matrix since the window size changed
	MakeProjectionMatrix();
}


Camera::~Camera()
{
}

XMFLOAT4X4 Camera::getProjectionMatrix()
{
	return projectionMatrix;
}

XMFLOAT4X4 Camera::getViewMatrix()
{
	return viewMatrix;
}

void Camera::setProjectionMatrix(XMFLOAT4X4 pro)
{
	projectionMatrix = pro;
}

void Camera::Update(float deltaTime)
{
	
	if (GetAsyncKeyState('W') & 0x8000) { 
		XMVECTOR directionVector = XMLoadFloat3(&direction);
		XMVECTOR positionVector = XMLoadFloat3(&position);
		XMVECTOR directionVectorNormal = XMVector3Normalize(directionVector);
		XMStoreFloat3(&position, (positionVector + (directionVectorNormal*deltaTime)));
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		XMVECTOR directionVector = XMLoadFloat3(&direction);
		XMVECTOR positionVector = XMLoadFloat3(&position);
		XMVECTOR directionVectorNormal = XMVector3Normalize(directionVector);
		XMStoreFloat3(&position, (positionVector - (directionVectorNormal*deltaTime)));
	}

	if (GetAsyncKeyState('A') & 0x8000) {
		XMVECTOR directionVector = XMLoadFloat3(&direction);
		XMVECTOR positionVector = XMLoadFloat3(&position);
		XMVECTOR up = { 0, 1, 0, 0 };
		XMVECTOR directionVectorNormal = XMVector3Normalize(directionVector);
		XMStoreFloat3(&position, (positionVector + (XMVector3Cross(directionVectorNormal, up) * deltaTime)));
	}

	if (GetAsyncKeyState('D') & 0x8000) {
		XMVECTOR directionVector = XMLoadFloat3(&direction);
		XMVECTOR positionVector = XMLoadFloat3(&position);
		XMVECTOR up = { 0, 1, 0, 0 };
		XMVECTOR directionVectorNormal = XMVector3Normalize(directionVector);
		XMStoreFloat3(&position, (positionVector - (XMVector3Cross(directionVectorNormal, up) * deltaTime)));
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) { position.y += 1.f * deltaTime; }
	if (GetAsyncKeyState('X') & 0x8000) { position.y -= 1.f * deltaTime; }
	
	MakeViewMatrix();
}

void Camera::MoveByMouse(float x, float y)
{
	XMVECTOR rotationVector = XMQuaternionRotationRollPitchYaw(y, x, 0.0f);
	XMVECTOR directionVector = XMVector3Rotate(XMLoadFloat3(&direction), rotationVector);
	XMStoreFloat3(&direction, directionVector);

	MakeViewMatrix();
}