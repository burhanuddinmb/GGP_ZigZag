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
#include "Emitter.h"
#include "SpriteBatch.h"
#include <Windows.h>
#include <mmsystem.h>
#include <windows.h>
#include <iostream>


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

	//Create environmental objects
	void SpawnEnvObjects();
	void SpawnVenus();
	void SpawnTimer(float deltaTime);
	void SpawnTimerPlanets(float deltaTime);

	//To create paths
	void CreatePath();
	void CreatePlankStraight(Material* material);
	void CreatePlankLeft(Material* material);
	void CheckIfNeedToRemovePlanks();
	void EnableBlending();
	//To run game
	void MoveBallOnPlatform(float deltaTime);
	void CheckPhysics();
	void CreateParticles();

	//SpriteBatch
	void InitializeSpriteBatch();


	Camera* camera;
	float timer = 0.0f;
	float timer1 = 8.0f;

	DirectionalLight sun;
	DirectionalLight sun2;
	bool timeToCreate;
	float timerCurrent = 0.0f;
	float timerTotal = 2.0f;

	Mesh *asteroid;
	Mesh *venus;
	Mesh *pluto;
	Mesh *neptune;

	//SkyBox
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;

	//PostProcessing
	ID3D11RenderTargetView* postProcessingRenderTarget;
	ID3D11ShaderResourceView* postProcessingSRV;
	SimpleVertexShader* postProcessingVS;
	SimplePixelShader* postProcessingPS;

	// Sky render states
	ID3D11RasterizerState* skyRastState;
	ID3D11DepthStencilState* skyDepthState;

	//Other Samplers
	ID3D11SamplerState* sampler1;
	D3D11_SAMPLER_DESC sampleData1;
	ID3D11ShaderResourceView* SRV1;

	ID3D11SamplerState* sampler2;
	D3D11_SAMPLER_DESC sampleData2;
	ID3D11ShaderResourceView* SRV2;

	//Sky Sampler
	ID3D11SamplerState* sampler3;
	D3D11_SAMPLER_DESC sampleData3;
	ID3D11ShaderResourceView* skySRV;
	GameEntity* skyBox;

	//Other Samplers
	ID3D11SamplerState* sampler4;
	D3D11_SAMPLER_DESC sampleData4;
	ID3D11ShaderResourceView* SRV4;
	
	ID3D11SamplerState* sampler5;
	D3D11_SAMPLER_DESC sampleData5;
	ID3D11ShaderResourceView* SRV5;

	ID3D11SamplerState* sampler6;
	D3D11_SAMPLER_DESC sampleData6;
	ID3D11ShaderResourceView* SRV6;

	ID3D11SamplerState* sampler7;
	D3D11_SAMPLER_DESC sampleData7;
	ID3D11ShaderResourceView* SRV7;

	//Path Samplers
	ID3D11SamplerState* samplerWater;
	D3D11_SAMPLER_DESC sampleDataWater;
	ID3D11ShaderResourceView* SRVWater;
	ID3D11ShaderResourceView* SRVWaterNormal;

	ID3D11SamplerState* samplerLava;
	D3D11_SAMPLER_DESC sampleDataLava;
	ID3D11ShaderResourceView* SRVLava;
	ID3D11ShaderResourceView* SRVLavaNormal;

	ID3D11SamplerState* samplerSand;
	D3D11_SAMPLER_DESC sampleDataSand;
	ID3D11ShaderResourceView* SRVSand;
	ID3D11ShaderResourceView* SRVSandNormal;

	//The new Mesh objects
	std::vector<Mesh*> meshObjects;
	std::vector<GameEntity*> gameObjects;
	std::vector<Material*> materialObjects;
	std::vector<GameEntity*> envObjects;
	std::vector<Material*>envMaterials;
	std::vector<GameEntity*> planetObjects;
	std::vector<Material*> planetMaterials;

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
	GameMode currentGameMode = start;

	//Particles
	EmitterColor currentEmitterColor;
	ID3D11ShaderResourceView* particleTexture;
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;
	ID3D11DepthStencilState* particleDepthState;
	ID3D11BlendState* particleBlendState;
	Emitter* emitter;

	//SPriteBatch
	//Main Menu and end game
	DirectX::SpriteBatch* spriteBatch;
	ID3D11ShaderResourceView* SRVTitle;
	ID3D11ShaderResourceView* SRVStart1;
	ID3D11ShaderResourceView* SRVStart2;
	ID3D11ShaderResourceView* SRVPaused;
	ID3D11ShaderResourceView* SRVGameOver;
	ID3D11ShaderResourceView* SRVCredits;
	ID3D11ShaderResourceView* SRVEscape;

	float time = 0.0f;
	float gameOverCreditsTimer = 0.0f;
	float alphaForEsc = 0.0f;
	bool increasingAlphaForEsc = true;
	ID3D11ShaderResourceView* SRVVariableStartDisplay;
	RECT titleRect;
	RECT startRect;
	RECT pauseRect;
	RECT gameOverRect;
	RECT creditsRect;
	RECT escRect;

	unsigned int height1ToCheck;
	unsigned int height2ToCheck;
	unsigned int width1ToCheck;
	unsigned int width2ToCheck;
	//Let's see if retry needs to be implemented
};

