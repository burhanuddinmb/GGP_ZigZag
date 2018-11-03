#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include "Vertex.h"
#include <Windows.h>
#include "DXCore.h"

using namespace DirectX;

enum GameMode {
	start,
	inGame,
	pause,
	gameOver
};

class Camera
{
public:
	Camera(int width, int height);
	~Camera();
	
	XMFLOAT4X4 getProjectionMatrix();
	XMFLOAT4X4 getViewMatrix();
	void setProjectionMatrix(XMFLOAT4X4);
	void OnResize(int width, int height);
	void Update(float deltaTime, XMFLOAT3 ballPosition);
	void MoveByMouse(float x, float y);
	void MoveRelative(float x, float y, float z);
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetDirection();
	void ChangeCameraPosition();
	void SetGameMode(GameMode gameMode);
private:
	void MakeProjectionMatrix();
	void MakeViewMatrix();
	void MakeCameraFaceBall(XMFLOAT3 ballPosition);
	bool LerpCamera(XMFLOAT3 ballPosition, float deltaTime);

	XMFLOAT3 position;

	XMFLOAT3 positionMove;

	XMFLOAT3 direction;
	XMFLOAT3 offset;
	XMFLOAT3 offsetRight;
	XMFLOAT3 offsetLeft;
	XMFLOAT3 directionleft;
	int width;
	int height;
	bool isTurningLeft;
	int counter = 0;
	bool isLerping = true;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	GameMode currentGameMode;
	XMFLOAT3 finalBallPosition;
	float multiplier = 1.0f;
};

