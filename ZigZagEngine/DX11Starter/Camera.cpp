#include "Camera.h"

Camera::Camera(int width, int height)
{
	direction = XMFLOAT3(-1.0f, 0.0f, 1.0f);
	directionleft = XMFLOAT3(-0.4f, 0.0f, 1.0f);
	position = XMFLOAT3(3.0f, 3.5f, -7.5f);

	offsetRight = XMFLOAT3(3.0f, 3.5f, -7.5f);
	offsetLeft = XMFLOAT3(6.0f, 3.5f, -4.5f);
	offset = offsetRight;
	isTurningLeft = false;
	//positionMove = XMFLOAT3(2.0f, 1.5f, -6.0f);
	currentGameMode = start;
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

void Camera::MakeCameraFaceBall(XMFLOAT3 ballPosition)
{
	direction = XMFLOAT3(ballPosition.x - position.x, ballPosition.y - position.y, ballPosition.z - position.z);
}

bool Camera::LerpCamera(XMFLOAT3 ballPosition, float deltaTime)
{
	//offsetRight = XMFLOAT3(3.0f, 3.5f, -7.5f);
	//offsetLeft = XMFLOAT3(6.0f, 3.5f, -4.5f);
	bool xNotDone = true;
	bool zNotDone = true;
	if (isTurningLeft)
	{
		//Right to left
		position = XMFLOAT3(position.x + 1.0f*deltaTime, ballPosition.y + offset.y, position.z  + 3.5f*deltaTime);
		if (position.x >= ballPosition.x + offset.x)
		{
			position.x = ballPosition.x + offset.x;
			xNotDone = false;
		}
		if (position.z >= ballPosition.z + offset.z)
		{
			position.z = ballPosition.z + offset.z;
			zNotDone = false;
		}
	}
	else
	{
		//Left to Right
		position = XMFLOAT3(position.x - 3.5f*deltaTime, ballPosition.y + offset.y, position.z - 1.0f*deltaTime);
		if (position.x <= ballPosition.x + offset.x)
		{
			position.x = ballPosition.x + offset.x;
			xNotDone = false;
		}
		if (position.z <= ballPosition.z + offset.z)
		{
			position.z = ballPosition.z + offset.z;
			zNotDone = false;
		}
	}
	return (xNotDone || zNotDone);
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

void Camera::MoveRelative(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	MakeViewMatrix();
}

XMFLOAT3 Camera::GetPosition()
{
	return position;
}

XMFLOAT3 Camera::GetDirection()
{
	return direction;
}

void Camera::ChangeCameraPosition()
{
	isTurningLeft = !isTurningLeft;
	if (isTurningLeft)
	{
		offset = offsetLeft;
	}
	else
	{
		offset = offsetRight;
	}
	isLerping = true;
}

void Camera::SetGameMode(GameMode gameMode)
{
	currentGameMode = gameMode;
	isLerping = true;
}

void Camera::Update(float deltaTime, XMFLOAT3 ballPosition)
{
	/*
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
	*/
	/*
	if (leftTurnOn == 1 && counter < 4000) {
		XMVECTOR directionVector = XMLoadFloat3(&directionleft);
		XMVECTOR positionVector = XMLoadFloat3(&position);
		XMVECTOR up = { 0, 1, 0, 0 };
		XMVECTOR directionVectorNormal = XMVector3Normalize(directionVector);
		XMStoreFloat3(&position, (positionVector + (XMVector3Cross(directionVectorNormal, up) * deltaTime)));
		counter++;
	}

	//if ball turning right
	if (leftTurnOn == 2 && counter< 4000)
	{
		XMVECTOR directionVector = XMLoadFloat3(&directionleft);
		XMVECTOR positionVector = XMLoadFloat3(&position);
		XMVECTOR up = { 0, 1, 0, 0 };
		XMVECTOR directionVectorNormal = XMVector3Normalize(directionVector);
		XMStoreFloat3(&position, (positionVector - (XMVector3Cross(directionVectorNormal, up) * deltaTime)));
		counter++;
	}
	*/
	if (currentGameMode == inGame)
	{
		finalBallPosition = ballPosition;
		if (isLerping)
		{
			isLerping = LerpCamera(ballPosition, deltaTime);
		}
		else
		{
			position = XMFLOAT3(ballPosition.x + offset.x, ballPosition.y + offset.y, ballPosition.z + offset.z);
		}
		MakeCameraFaceBall(ballPosition);

	}
	else if (currentGameMode == start || currentGameMode == pause)
	{
		finalBallPosition = ballPosition;
		position = XMFLOAT3(position.x + (multiplier * deltaTime), ballPosition.y + offset.y, ballPosition.z + offset.z);
		if (position.x + deltaTime > ballPosition.x + offset.x + 2.0f)
		{
			multiplier = -1.0f;
		}
		else if (position.x + deltaTime < ballPosition.x + offset.x - 2.0f)
		{
			multiplier = 1.0f;
		}
		MakeCameraFaceBall(ballPosition);
	}
	else if (currentGameMode == gameOver)
	{
		position = XMFLOAT3(position.x + (multiplier * deltaTime), finalBallPosition.y + offset.y, finalBallPosition.z + offset.z);
		if (position.x + deltaTime > finalBallPosition.x + offset.x + 2.0f)
		{
			multiplier = -1.0f;
		}
		else if (position.x + deltaTime < finalBallPosition.x + offset.x - 2.0f)
		{
			multiplier = 1.0f;
		}
	}
	XMVECTOR directionVector = XMLoadFloat3(&directionleft);
	XMVECTOR positionVector = XMLoadFloat3(&position);
	XMVECTOR up = { 0, 1, 0, 0 };
	XMVECTOR directionVectorNormal = XMVector3Normalize(directionVector);
	XMStoreFloat3(&position, (positionVector - (XMVector3Cross(directionVectorNormal, up) * deltaTime)));
	MakeViewMatrix();
}

void Camera::MoveByMouse(float x, float y)
{
	/*XMVECTOR rotationVector = XMQuaternionRotationRollPitchYaw(y, x, 0.0f);
	XMVECTOR directionVector = XMVector3Rotate(XMLoadFloat3(&direction), rotationVector);
	XMStoreFloat3(&direction, directionVector);

	MakeViewMatrix();*/
}