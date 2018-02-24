#pragma once
#include "Mesh.h"
#include "Material.h"
#include <vector>
#include "Lights.h"

using namespace std;
using namespace DirectX;

class GameEntity
{
public:
	GameEntity(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, Mesh* inputMesh, Material* material);
	~GameEntity();
	//Getters
	XMFLOAT4X4 GetWorldMatrix();
	Mesh* GetMesh();

	//Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 position);

	void SetRotation(float x, float y, float z);
	void SetRotation(XMFLOAT3 rotation);

	void SetScale(float x, float y, float z);
	void SetScale(XMFLOAT3 scale);
	
	//Move functions
	void MoveRelative(float x, float y, float z);

	void RotateRelative(float x, float y, float z);

	void ResizeRelative(float x, float y, float z);

	void PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection);

private:

	void GenerateWorldMatrix();

	Mesh* mesh;
	Material* material;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMFLOAT4X4 worldMatrix;
	bool shouldGenerateWorldMatrix;
};

