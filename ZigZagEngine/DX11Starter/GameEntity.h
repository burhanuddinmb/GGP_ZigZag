#pragma once
#include "Mesh.h"
#include "Material.h"
#include <vector>
#include "Lights.h"
#include "Emitter.h"

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
	XMFLOAT3 GetPosition();

	void SetRotation(float x, float y, float z);
	void SetRotation(XMFLOAT3 rotation);
	XMFLOAT3 GetRotation();

	void SetScale(float x, float y, float z);
	void SetScale(XMFLOAT3 scale);
	XMFLOAT3 GetScale();
	
	//Move functions
	void MoveRelative(float x, float y, float z);

	void RotateRelative(float x, float y, float z);

	void ResizeRelative(float x, float y, float z);

	void PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection, float alpha = 1.0f); //defaulting alpha to 1.0f if no value is passed.

	void PrepareMaterialWater(XMFLOAT4X4 view, XMFLOAT4X4 projection, float time, int scrollNumber, float alpha = 1.0f);

	void Falling(float deltaTime, float gravity);

	bool TransitionPlankFromTopToPosition(XMFLOAT3 finalPosition, float deltaTime);

	Material* GetMaterial();

private:
	void GenerateWorldMatrix();
	Mesh* mesh;
	Material* material;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMFLOAT4X4 worldMatrix;
	bool shouldGenerateWorldMatrix;
	float gravity;
	float timeStep = 0.0f;
};

