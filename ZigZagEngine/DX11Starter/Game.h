#pragma once
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include <vector>
#include "GameEntity.h"
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include "WICTextureLoader.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShadersAndTextures(); 
	void CreateCamera();
	void CreateBasicGeometry();
	void CreateEntities();
	void LoadTheDirectionalLight();
	void InitialisingLocalVariables();

	//To create paths
	void CreatePath();
	void CreatePlankStraight();
	void CreatePlankLeft();
	void CheckIfNeedToRemovePlanks();
	void EnableBlending();
	//To run game
	void MoveBallOnPlatform(float deltaTime);
	void CheckPhysics();

	Camera* camera;

	DirectionalLight sun;
	DirectionalLight sun2;
	int timeToCheck = 1;

	ID3D11SamplerState* sampler1;
	D3D11_SAMPLER_DESC sampleData1;
	ID3D11ShaderResourceView* SRV1;

	ID3D11SamplerState* sampler2;
	D3D11_SAMPLER_DESC sampleData2;
	ID3D11ShaderResourceView* SRV2;

	//The new Mesh objects
	std::vector<Mesh*> meshObjects;
	std::vector<GameEntity*> gameObjects;
	std::vector<Material*> materialObjects;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.

	ID3D11RasterizerState* rsState;
	ID3D11BlendState* blendState;

	POINT prevMousePos;
	bool lastStraightCreated;
	bool plankBeingRemoved = false;
	XMFLOAT3 pathPosition;
	bool isBallDirectionLeft;
	bool isFalling;
	float gravity;
	float degreeRotation = 1.5708f; //90degrees
	float plankBeingPlaced = false;
	XMFLOAT3 finalPositionOfLatestPlankCreated;
	XMFLOAT3 finalPositionOfDeletingPlank;
};

