#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include "Vertex.h"
#include <Windows.h>
#include "DXCore.h"

using namespace DirectX;

class Camera
{
public:
	Camera(int width, int height);
	~Camera();
	
	XMFLOAT4X4 getProjectionMatrix();
	XMFLOAT4X4 getViewMatrix();
	void setProjectionMatrix(XMFLOAT4X4);
	void OnResize(int width, int height);
	void Update(float deltaTime);
	void MoveByMouse(float x, float y);
	void MoveRelative(float x, float y, float z);
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetDirection();

private:
	void MakeProjectionMatrix();
	void MakeViewMatrix();
	

	XMFLOAT3 position;
	XMFLOAT3 direction;
	int width;
	int height;

	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
};

