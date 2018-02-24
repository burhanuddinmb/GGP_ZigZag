#include "GameEntity.h"

GameEntity::GameEntity(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, Mesh* inputMesh, Material* material)
{
	SetPosition(position);
	SetScale(scale);
	SetRotation(rotation);
	this->material = material;
	mesh = inputMesh;
	GenerateWorldMatrix();
}

GameEntity::~GameEntity()
{
}

XMFLOAT4X4 GameEntity::GetWorldMatrix()
{
	if (shouldGenerateWorldMatrix)
	{
		GenerateWorldMatrix();
		shouldGenerateWorldMatrix = false;
	}
	return worldMatrix;
}

void GameEntity::SetPosition(float x, float y, float z)
{
	position = { x, y, z };
	shouldGenerateWorldMatrix = true;
}

void GameEntity::SetPosition(XMFLOAT3 position)
{
	this->position = position;
	shouldGenerateWorldMatrix = true;
}

void GameEntity::SetRotation(float x, float y, float z)
{
	rotation = { x, y, z };
	shouldGenerateWorldMatrix = true;
}

void GameEntity::SetRotation(XMFLOAT3 rotation)
{
	this->rotation = rotation;
	shouldGenerateWorldMatrix = true;
}

void GameEntity::SetScale(float x, float y, float z)
{
	scale = { x, y, z };
	shouldGenerateWorldMatrix = true;
}

void GameEntity::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
	shouldGenerateWorldMatrix = true;
}

void GameEntity::MoveRelative(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	shouldGenerateWorldMatrix = true;
}

void GameEntity::RotateRelative(float x, float y, float z)
{
	rotation.x += x;
	rotation.y += y;
	rotation.z += z;
	shouldGenerateWorldMatrix = true;
}

void GameEntity::ResizeRelative(float x, float y, float z)
{
	scale.x += x;
	scale.y += y;
	scale.z += z;
	shouldGenerateWorldMatrix = true;
}


void GameEntity::GenerateWorldMatrix()
{
	// Create the individual transformations
	//Individual Vectors
	XMVECTOR positionVector = XMLoadFloat3(&position);
	XMVECTOR rotationVector = XMLoadFloat3(&rotation);
	XMVECTOR scaleVector = XMLoadFloat3(&scale);

	// Individual Relative Matrices
	XMMATRIX positionMatrix = XMMatrixTranslationFromVector(positionVector);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(scaleVector);

	// Combine together
	// Same as calling XMMatrixMultiply twice
	XMMATRIX world = scaleMatrix * rotationMatrix * positionMatrix;

	//Our world
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
}

void GameEntity::PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	GenerateWorldMatrix();
	SimpleVertexShader *tempVertexShader = material->GetVertexShader();
	SimplePixelShader *tempPixelShader = material->GetPixelShader();

	tempVertexShader->SetMatrix4x4("world", worldMatrix);
	tempVertexShader->SetMatrix4x4("view", view);
	tempVertexShader->SetMatrix4x4("projection", projection);
	tempPixelShader->SetShaderResourceView("diffuseTexture", material->GetSRV());
	tempPixelShader->SetSamplerState("basicSampler", material->GetSampler());
	tempVertexShader->SetShader();
	tempPixelShader->SetShader();	

	tempVertexShader->CopyAllBufferData();
	tempPixelShader->CopyAllBufferData();


}

Mesh* GameEntity::GetMesh()
{
	return mesh;
}
