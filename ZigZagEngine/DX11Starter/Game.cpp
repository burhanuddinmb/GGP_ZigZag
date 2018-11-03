#include "Game.h"
#include "Vertex.h"
#include <d3d11.h>
#include <conio.h>
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include <ctime>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"ZigZag",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
	isBallDirectionLeft = true;
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class

	//Delete the Mesh objects
	for (uint16_t i = 0; i < meshObjects.size(); i++)
	{
		delete meshObjects[i];
		meshObjects[i] = nullptr;
	}
	meshObjects.clear();

	//Delete the Game Entity objects
	for (uint16_t i = 0; i < gameObjects.size(); i++)
	{
		delete gameObjects[i];
		gameObjects[i] = nullptr;
	}
	gameObjects.clear();

	//Delete the Material objects
	for (uint16_t i = 0; i < materialObjects.size(); i++)
	{
		delete materialObjects[i];
		materialObjects[i] = nullptr;
	}
	materialObjects.clear();

	//Delete the Environmental objects
	for (uint16_t i = 0; i < envObjects.size(); i++)
	{
		delete envObjects[i];
		envObjects[i] = nullptr;
	}
	envObjects.clear();

	//Delete the Environmental materials
	for (uint16_t i = 0; i < envMaterials.size(); i++)
	{
		delete envMaterials[i];
		envMaterials[i] = nullptr;
	}
	envMaterials.clear();

	//Delete venus material
	for (uint16_t i = 0; i < planetMaterials.size(); i++)
	{
		delete planetMaterials[i];
		planetMaterials[i] = nullptr;
	}
	planetMaterials.clear();

	//Delete venus objects
	for (uint16_t i = 0; i < planetObjects.size(); i++)
	{
		delete planetObjects[i];
		planetObjects[i] = nullptr;
	}
	planetObjects.clear();

	//Delete the Environment Mesh
	delete asteroid;
	delete venus;


	if (rsState != nullptr) rsState->Release();

	if (blendState != nullptr) blendState->Release();

	SRVTitle->Release();
	SRVStart2->Release();
	SRVStart1->Release();
	SRVPaused->Release();
	SRVGameOver->Release();
	SRVCredits->Release();
	SRVEscape->Release();
	delete spriteBatch;

	// Clean up our other resources
	skySRV->Release();
	skyDepthState->Release();
	skyRastState->Release();
	sampler3->Release();
	SRVLavaNormal->Release();
	SRVSandNormal->Release();
	SRVWaterNormal->Release();
	delete skyBox;
	delete camera;
	delete skyVS;
	delete skyPS;

	//Deleting particle objects
	particleTexture->Release();
	particleBlendState->Release();
	particleDepthState->Release();

	delete emitter;
	delete particleVS;
	delete particlePS;

	//PostProcessing
	postProcessingRenderTarget->Release();
	postProcessingSRV->Release();
	delete postProcessingVS;
	delete postProcessingPS;
	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	InitialisingLocalVariables();
	EnableBlending();
	// Makes the controllable entities here
	CreateEntities();
	CreateParticles();

	//Loops the Audio
	//PlaySound(TEXT("SpaceAudio.wav"), NULL, SND_LOOP | SND_ASYNC);
	PlaySound(TEXT("../../Assets/Audios/RollingSpace.wav"), NULL, SND_LOOP | SND_ASYNC);

	InitializeSpriteBatch();
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Game::CreateParticles()
{
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/particle.jpg", 0, &particleTexture);

	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepthState);


	// Blend for particles (additive)
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	// Set up particles
	emitter = new Emitter(
		400,							// Max particles
		40,							// Particles per second
		3,								// Particle lifetime
		0.1f,							// Start size
		2.0f,							// End size
		//Water
		/*XMFLOAT4(0.1f, 0.1f, 1.0f, 0.2f),	// Start color
		XMFLOAT4(0.1f, 0.6f, 1.0f, 0.0f),		// End color*/
		//Fire
		XMFLOAT4(0.1f, 0.1f, 1.0f, 0.2f),	// Start color
		XMFLOAT4(0.1f, 0.6f, 1.0f, 0.0f),		// End color
		/*XMFLOAT4(0.1f, 0.1f, 1.0f, 0.2f),	// Start color
		XMFLOAT4(0.1f, 0.6f, 1.0f, 0.0f),		// End color*/
		//To set direction
		XMFLOAT3(0.0f, 1.2f, -1.5f),				// Start velocity
		//XMFLOAT3(1.5f, 1.2f, 0.0f),
		XMFLOAT3(2.0f, 0.0f, 0.0f),				// Start position
		XMFLOAT3(0.0f, -0.6f, 0.0f),				// Start acceleration
		//XMFLOAT3(0.6f, 0.0f, 0.0f),
		device,
		particleVS,
		particlePS,
		particleTexture);
}

void Game::InitializeSpriteBatch()
{
	//Initialize sprite batch
	spriteBatch = new SpriteBatch(context);
	//Make their individual materials
	CreateWICTextureFromFile(device, context, L"../../Assets/Sprites/Title.png", 0, &SRVTitle);
	CreateWICTextureFromFile(device, context, L"../../Assets/Sprites/Start1.png", 0, &SRVStart1);
	CreateWICTextureFromFile(device, context, L"../../Assets/Sprites/Start2.png", 0, &SRVStart2);
	CreateWICTextureFromFile(device, context, L"../../Assets/Sprites/Paused.png", 0, &SRVPaused);
	CreateWICTextureFromFile(device, context, L"../../Assets/Sprites/GameOver.png", 0, &SRVGameOver);
	CreateWICTextureFromFile(device, context, L"../../Assets/Sprites/Credits.png", 0, &SRVCredits);
	CreateWICTextureFromFile(device, context, L"../../Assets/Sprites/PressESC.png", 0, &SRVEscape);
	SRVVariableStartDisplay = SRVStart2;

	// Create the Rects to house the materials in
	startRect = { (LONG)(width / 2) - 120, (LONG)(height / 2) + 120, (LONG)(width / 2) + 100 , (LONG)(height / 2) + 180 };

	titleRect = { (LONG)(width / 2) - 230, (LONG)(height / 2) - 250, (LONG)(width / 2) + 200 , (LONG)(height / 2) -150 };

	pauseRect = { (LONG)(width / 2) - 140, (LONG)(height / 2) - 180, (LONG)(width / 2) + 140 , (LONG)(height / 2) -100 };

	gameOverRect = { (LONG)(width / 2) - 250, (LONG)(height / 2) - 250, (LONG)(width / 2) + 250 , (LONG)(height / 2) - 150 };

	creditsRect = { 100, (LONG)(height / 2) - 200, (LONG)width - 100 , (LONG)height - 40 };

	escRect = { (LONG)width - 270,(LONG)height - 60,  (LONG)width - 10,(LONG)height - 10  };

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShadersAndTextures()
{
	//Creating texture1
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/paper.jpeg", 0, &SRV1);
	
	sampleData1 = {};
	sampleData1.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData1.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData1.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData1.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleData1.MaxAnisotropy = 16;
	sampleData1.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleData1, &sampler1);

	//These get deleted in material
	SimpleVertexShader *vertexShader1 = new SimpleVertexShader(device, context);
	vertexShader1->LoadShaderFile(L"VertexShader.cso");

	SimplePixelShader *pixelShader1 = new SimplePixelShader(device, context);
	pixelShader1->LoadShaderFile(L"PixelShader.cso");

	pixelShader1->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShader1->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	//Creating texture2
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/earth.jpeg", 0, &SRV2);

	sampleData2 = {};
	sampleData2.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData2.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData2.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData2.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleData2.MaxAnisotropy = 16;
	sampleData2.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleData2, &sampler2);

	SimpleVertexShader *vertexShader2 = new SimpleVertexShader(device, context);
	vertexShader2->LoadShaderFile(L"VertexShader.cso");

	SimplePixelShader *pixelShader2 = new SimplePixelShader(device, context);
	pixelShader2->LoadShaderFile(L"PixelShader.cso");

	pixelShader2->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShader2->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	//Creating a texture for env object
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/Asteroid.jpg", 0, &SRV4);

	sampleData4 = {};
	sampleData4.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData4.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData4.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData4.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleData4.MaxAnisotropy = 16;
	sampleData4.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleData4, &sampler4);

	SimpleVertexShader *vertexShader4 = new SimpleVertexShader(device, context);
	vertexShader4->LoadShaderFile(L"VertexShader.cso");

	SimplePixelShader *pixelShader4 = new SimplePixelShader(device, context);
	pixelShader4->LoadShaderFile(L"PixelShader.cso");

	pixelShader4->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShader4->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	//Creating Skybox
	CreateDDSTextureFromFile(device, context, L"../../Assets/Materials/Spaceskybox2.dds", 0, &skySRV);

	skyVS = new SimpleVertexShader(device, context);
	skyVS->LoadShaderFile(L"SkyVertexShader.cso");

	skyPS = new SimplePixelShader(device, context);
	skyPS->LoadShaderFile(L"SkyPixelShader.cso");

	// Create a sampler state that holds options for sampling
	// The descriptions should always just be local variables
	D3D11_SAMPLER_DESC samplerDesc = {}; 
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Setting this allows for mip maps to work! (if they exist)

	// Ask DirectX for the actual object
	device->CreateSamplerState(&samplerDesc, &sampler3);

	// Create states for sky rendering
	D3D11_RASTERIZER_DESC rs = {};
	rs.CullMode = D3D11_CULL_FRONT;
	rs.FillMode = D3D11_FILL_SOLID;
	device->CreateRasterizerState(&rs, &skyRastState);

	D3D11_DEPTH_STENCIL_DESC ds = {};
	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&ds, &skyDepthState);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Eventually put everything in the material
	// *** NOTICE ***
	//The variables that we are passing, get deleted in material
	//We shall change implementation if needed
	materialObjects.push_back(new Material(vertexShader1, pixelShader1, SRV1, sampler1));
	materialObjects.push_back(new Material(vertexShader2, pixelShader2, SRV2, sampler2));

	//particle shaders
	particleVS = new SimpleVertexShader(device, context);
	particleVS->LoadShaderFile(L"ParticleEmitterVS.cso");

	particlePS = new SimplePixelShader(device, context);
	particlePS->LoadShaderFile(L"ParticleEmitterPS.cso");

	//Create plank material
	//Creating texture1
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/water.jpg", 0, &SRVWater);

	//Added normal map
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/waterNormal2.jpg", 0, &SRVWaterNormal);

	sampleDataWater = {};
	sampleDataWater.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataWater.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataWater.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataWater.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleDataWater.MaxAnisotropy = 16;
	sampleDataWater.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleDataWater, &samplerWater);

	//These get deleted in material
	//Made changes here to water shader
	SimpleVertexShader *vertexShaderWater = new SimpleVertexShader(device, context);
	vertexShaderWater->LoadShaderFile(L"VertexShaderWater.cso");

	SimplePixelShader *pixelShaderWater = new SimplePixelShader(device, context);
	pixelShaderWater->LoadShaderFile(L"PixelShaderWater.cso");

	pixelShaderWater->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShaderWater->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	//Creating texture1
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/sand.jpg", 0, &SRVSand);

	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/sandNormal.jpg", 0, &SRVSandNormal);

	sampleDataSand = {};
	sampleDataSand.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataSand.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataSand.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataSand.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleDataSand.MaxAnisotropy = 16;
	sampleDataSand.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleDataSand, &samplerSand);

	//These get deleted in material
	SimpleVertexShader *vertexShaderSand = new SimpleVertexShader(device, context);
	vertexShaderSand->LoadShaderFile(L"VertexShaderWater.cso");

	SimplePixelShader *pixelShaderSand = new SimplePixelShader(device, context);
	pixelShaderSand->LoadShaderFile(L"PixelShaderWater.cso");

	pixelShaderSand->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShaderSand->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	//Creating texture1
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/lava.jpg", 0, &SRVLava);

	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/lavaNormal.jpg", 0, &SRVLavaNormal);

	sampleDataLava = {};
	sampleDataLava.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataLava.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataLava.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDataLava.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleDataLava.MaxAnisotropy = 16;
	sampleDataLava.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleDataLava, &samplerLava);

	//These get deleted in material
	SimpleVertexShader *vertexShaderLava = new SimpleVertexShader(device, context);
	vertexShaderLava->LoadShaderFile(L"VertexShaderWater.cso");

	SimplePixelShader *pixelShaderLava = new SimplePixelShader(device, context);
	pixelShaderLava->LoadShaderFile(L"PixelShaderWater.cso");

	pixelShaderLava->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShaderLava->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	materialObjects.push_back(new Material(vertexShaderWater, pixelShaderWater, SRVWater, samplerWater, water)); //2
	materialObjects.push_back(new Material(vertexShaderSand, pixelShaderSand, SRVSand, samplerSand, earth)); //3
	materialObjects.push_back(new Material(vertexShaderLava, pixelShaderLava, SRVLava, samplerLava, fire)); //4
	envMaterials.push_back(new Material(vertexShader4, pixelShader4, SRV4, sampler4));
	//Postprocessing Bloom
	postProcessingVS = new SimpleVertexShader(device, context);
	postProcessingVS->LoadShaderFile(L"PostProcessVertexShader.cso");

	postProcessingPS = new SimplePixelShader(device, context);
	postProcessingPS->LoadShaderFile(L"PostProcessPixelShader.cso");
	

	//Creating Venus texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/venus.jpg", 0, &SRV5);

	sampleData5 = {};
	sampleData5.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData5.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData5.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData5.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleData5.MaxAnisotropy = 16;
	sampleData5.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleData5, &sampler5);

	SimpleVertexShader *vertexShader5 = new SimpleVertexShader(device, context);
	vertexShader5->LoadShaderFile(L"VertexShader.cso");

	SimplePixelShader *pixelShader5 = new SimplePixelShader(device, context);
	pixelShader5->LoadShaderFile(L"PixelShader.cso");

	pixelShader5->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShader5->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	planetMaterials.push_back(new Material(vertexShader5, pixelShader5, SRV5, sampler5));
	
	//Creating Neptune texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/neptune.jpg", 0, &SRV7);

	sampleData7 = {};
	sampleData7.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData7.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData7.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData7.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleData7.MaxAnisotropy = 16;
	sampleData7.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleData7, &sampler7);

	SimpleVertexShader *vertexShader7 = new SimpleVertexShader(device, context);
	vertexShader7->LoadShaderFile(L"VertexShader.cso");

	SimplePixelShader *pixelShader7 = new SimplePixelShader(device, context);
	pixelShader7->LoadShaderFile(L"PixelShader.cso");

	pixelShader7->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShader7->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	planetMaterials.push_back(new Material(vertexShader7, pixelShader7, SRV7, sampler7));

	//Creating Pluto texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/pluto.jpg", 0, &SRV6);

	sampleData6 = {};
	sampleData6.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData6.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData6.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleData6.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleData6.MaxAnisotropy = 16;
	sampleData6.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampleData6, &sampler6);

	SimpleVertexShader *vertexShader6 = new SimpleVertexShader(device, context);
	vertexShader6->LoadShaderFile(L"VertexShader.cso");

	SimplePixelShader *pixelShader6 = new SimplePixelShader(device, context);
	pixelShader6->LoadShaderFile(L"PixelShader.cso");

	pixelShader6->SetData(
		"sun",  // The name of the (eventual) variable in the shader
		&sun,   // The address of the data to copy
		sizeof(DirectionalLight));

	pixelShader6->SetData(
		"sun2",  // The name of the (eventual) variable in the shader
		&sun2,   // The address of the data to copy
		sizeof(DirectionalLight));

	planetMaterials.push_back(new Material(vertexShader6, pixelShader6, SRV6, sampler6));
}


// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateCamera()
{
	//Making camera here
	camera = new Camera(width, height);
}


// --------------------------------------------------------
// Creates the geometry we're going to draw here. Shapes!
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable

	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	XMFLOAT2 uv = XMFLOAT2(0.0f, 0.0f);
	XMFLOAT3 tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Vertex triangleVertices[] = 
	{
		{ XMFLOAT3(-1.0f, +0.0f, +0.0f), uv, normal , tangent},
		{ XMFLOAT3(+0.0f, +1.0f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(+1.0f, +0.0f, +0.0f), uv, normal , tangent },
	};

	int triangleIndices[] = { 0,1,2 };

	Mesh *triangle = new Mesh(
		triangleVertices,
		(sizeof(triangleVertices) / (sizeof(float) * 7)),
		triangleIndices,
		sizeof(triangleIndices) / sizeof(int),
		device);


	//Creating a square in the centre
	Vertex squareVertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, +0.0f), uv, normal , tangent  },
		{ XMFLOAT3(-1.0f, +1.0f, +0.0f), uv, normal , tangent  },
		{ XMFLOAT3(+1.0f, +1.0f, +0.0f), uv, normal , tangent  },
		{ XMFLOAT3(+1.0f, -1.0f, +0.0f), uv, normal , tangent  },
	};

	int squareIndices[] = { 0,1,2 , 0,2,3};

	Mesh *square = new Mesh(
		squareVertices,
		(sizeof(squareVertices) / (sizeof(float) * 7)),
		squareIndices,
		sizeof(squareIndices) / sizeof(int),
		device);

	//Creating a pentagon on the right
	Vertex pentagonVertices[] =
	{
		{ XMFLOAT3(+1.2f, -0.8f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(+1.2f, +0.8f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(+2.2f, +1.2f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(+3.0f, +0.0f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(+2.2f, -1.2f, +0.0f), uv, normal , tangent },
	};
	int pentagonIndices[] = { 0,1,4 , 1,2,3 , 1,3,4};

	Mesh *pentagon = new Mesh(
		pentagonVertices,
		(sizeof(pentagonVertices) / (sizeof(float) * 7)),
		pentagonIndices,
		sizeof(pentagonIndices) / sizeof(int),
		device);

	
	//Create a CNC look alike image
	Vertex cncVertices[] =
	{
		{ XMFLOAT3(-3.4f, -1.5f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-3.4f, +1.5f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-1.1f, +1.5f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-1.1f, +0.0f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-1.5f, -0.5f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-1.9f, -0.0f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-1.9f, +0.2f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-2.6f, +0.2f, +0.0f), uv, normal , tangent },
		{ XMFLOAT3(-2.6f, -1.5f, +0.0f), uv, normal , tangent },
	};
	int cncIndices[] = { 0,1,7 , 1,6,7 , 0,7,8 ,  1,2,6 , 6,2,3 , 5,3,4 , 6,3,5};

	
	Mesh *cnc = new Mesh(
		cncVertices,
		(sizeof(cncVertices) / (sizeof(float) * 7)),
		cncIndices,
		sizeof(cncIndices) / sizeof(int),
		device);
		
	
	Mesh *torus = new Mesh("../../Assets/Models/torus.obj", device);
	Mesh *helix = new Mesh("../../Assets/Models/helix.obj", device);
	Mesh *sphere = new Mesh("../../Assets/Models/sphere.obj", device);
	Mesh *cube = new Mesh("../../Assets/Models/cube.obj", device);
	//Push objects in vector	
	meshObjects.push_back(triangle);	//0
	meshObjects.push_back(square);		//1
	meshObjects.push_back(pentagon);	//2
	meshObjects.push_back(cnc);			//3
	meshObjects.push_back(torus);		//4
	meshObjects.push_back(helix);		//5
	meshObjects.push_back(sphere);		//6
	meshObjects.push_back(cube);		//7	

	//Mesh for Asteroid
	asteroid = new Mesh("../../Assets/Models/Asteroid.obj", device);

	//Mesh for Planets
	venus = new Mesh("../../Assets/Models/venus.obj", device);
}
void Game::CreateEntities()
{
	//Common positions for now
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(0.8f, 0.8f, 0.8f);

	//GameEntity *entity1 = new GameEntity(position, rotation, scale, meshObjects[0], materialObjects[0]);
	//GameEntity *entity2 = new GameEntity(position, rotation, scale, meshObjects[5], materialObjects[0]);
	//GameEntity *entity3 = new GameEntity(position, rotation, scale, meshObjects[4], materialObjects[0]);
	//GameEntity *entity4 = new GameEntity(position, rotation, scale, meshObjects[4], materialObjects[0]);
	GameEntity *entity5 = new GameEntity(position, rotation, scale, meshObjects[6], materialObjects[1]);
	skyBox = new GameEntity(position, rotation, scale, meshObjects[7], materialObjects[1]);
	//gameObjects.push_back(entity1);
	//gameObjects.push_back(entity2);
	//gameObjects.push_back(entity3);
	//gameObjects.push_back(entity4);
	gameObjects.push_back(entity5);

	//Put the two planks
	CreatePlankStraight(materialObjects[(rand() % 3) + 2]);
	CreatePlankStraight(materialObjects[(rand() % 3) + 2]);

	//JASON - Randomly place env object
	SpawnEnvObjects();

	//Correct position
	XMFLOAT3 tmpPosition;
	for (int i = 1; i < gameObjects.size(); i++)
	{
		tmpPosition = gameObjects[i]->GetPosition();
		tmpPosition.y -= 2.0f;
		gameObjects[i]->SetPosition(tmpPosition);
	}
	plankBeingPlaced = false;
	SpawnVenus();
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	camera->OnResize(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	time += deltaTime;

	const int KEY_UP = 0x1;
	camera->Update(deltaTime, gameObjects[0]->GetPosition());
	if ((GetAsyncKeyState('P') & KEY_UP) == KEY_UP)
	{
		if (currentGameMode == inGame)
		{
			currentGameMode = pause;
			camera->SetGameMode(pause);
		}
		else if (currentGameMode == pause)
		{
			currentGameMode = inGame;
			camera->SetGameMode(inGame);
		}
	}
	if (currentGameMode == inGame)
	{
		SpawnTimer(deltaTime); //JASON - Update loop controls SpawnTimer function
		SpawnTimerPlanets(deltaTime);

		emitter->Update(deltaTime, gameObjects[0]->GetPosition());
		MoveBallOnPlatform(deltaTime);
		CheckPhysics();
		if (timeToCreate)
		{
			timeToCreate = false;
			CreatePath();
		}

		for (int i = 0; i < planetObjects.size(); i++)
		{
			float angle = sin(.08f * deltaTime);
			planetObjects[i]->RotateRelative(0.0f, angle, 0.0f);
		}

		for (int i = 0; i < envObjects.size(); i++)
		{
			float angle = sin(.18f * deltaTime);
			envObjects[i]->RotateRelative(angle, 0.0f, 0.0f);
		}

		//New plank
		if (plankBeingPlaced)
		{
			plankBeingPlaced = gameObjects[gameObjects.size() - 1]->TransitionPlankFromTopToPosition(finalPositionOfLatestPlankCreated, deltaTime);
		}

		//Removing old plank
		if (plankBeingRemoved)
		{
			plankBeingRemoved = gameObjects[1]->TransitionPlankFromTopToPosition(finalPositionOfDeletingPlank, deltaTime);
			if (!plankBeingRemoved)
			{
				delete gameObjects[1];
				gameObjects.erase(gameObjects.begin() + 1);
			}
		}
		//Change ball rotation on key-press
		//NEEDS TO BE FIXED!
		if (!isFalling && (GetAsyncKeyState(VK_SPACE) & KEY_UP) == KEY_UP)
		{
			camera->ChangeCameraPosition();
			emitter->ChangeDirection();
			if (isBallDirectionLeft)
			{
				gameObjects[0]->RotateRelative(0.0f, -degreeRotation, 0.0f);
			}
			else
			{
				gameObjects[0]->RotateRelative(0.0f, +degreeRotation, 0.0f);
			}
			isBallDirectionLeft = !isBallDirectionLeft;
		}

		if (GetAsyncKeyState(VK_RETURN))
		{

#if defined(DEBUG) || defined(_DEBUG)
			//Print camera position to implement
			XMFLOAT3 cPosition = camera->GetPosition();
			XMFLOAT3 cDirection = camera->GetDirection();
			XMFLOAT3 bPosition = camera->GetPosition();
			printf("Camera position x: %f  y: %f z: %f \n", cPosition.x, cPosition.y, cPosition.z);
			printf("Camera direction x: %f  y: %f z: %f \n", cDirection.x, cDirection.y, cDirection.z);
			printf("Ball position x: %f  y: %f z: %f \n", bPosition.x, bPosition.y, bPosition.z);
#endif
		}

	}
	else if (currentGameMode == gameOver)
	{
		gameOverCreditsTimer += deltaTime;
		MoveBallOnPlatform(deltaTime);
		if (gameOverCreditsTimer > 1.0f)
		{
			if (increasingAlphaForEsc)
			{
				alphaForEsc += deltaTime;
				if (alphaForEsc > 1.0f)
				{
					increasingAlphaForEsc = false;
					alphaForEsc = 1.0f;
				}
			}
			else
			{
				alphaForEsc -= deltaTime;
				if (alphaForEsc < 0.0f)
				{
					increasingAlphaForEsc = true;
					alphaForEsc = 0.0f;
				}
			}
		}
	}
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{

	// Background color (Cornflower Blue in this case) for clearing
	//const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	//const float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	const float color[4] = { 0,0,0,0 };
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(postProcessingRenderTarget, color); // Clear the post process target too!

	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	if (currentGameMode != inGame)
	{
		context->OMSetRenderTargets(1, &postProcessingRenderTarget, depthStencilView);
	}
	//Drawing objects
	for (uint16_t i = 0; i < gameObjects.size(); i++)
	{
		//Not drawing transparent objects first
		if ((plankBeingPlaced && (i == (gameObjects.size() - 1))) || (plankBeingRemoved && i == 1))
		{
			continue;
		}

		//gameObjects[i]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix());

		if (gameObjects[i]->GetScale().x > 1.0f)
		{
			gameObjects[i]->PrepareMaterialWater(camera->getViewMatrix(), camera->getProjectionMatrix(), time, 1);
		}
		else
		{
			gameObjects[i]->PrepareMaterialWater(camera->getViewMatrix(), camera->getProjectionMatrix(), time, 0);
		}


		//Creating the Mesh objects
		// Set buffers in the input assembler
		//  - Do this ONCE PER OBJECT you're drawing, since each object might
		//    have different geometry.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11Buffer* vertexBufferPtr = gameObjects[i]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
		context->IASetIndexBuffer(gameObjects[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		// Finally do the actual drawing
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		context->DrawIndexed(
			gameObjects[i]->GetMesh()->GetIndexCount(),   // The number of indices to use (we could draw a subset if we wanted)
			0,							// Offset to the first index we want to use
			0);
	}

	for (uint16_t i = 0; i < envObjects.size(); i++)
	{
		//Not drawing transparent objects first

		envObjects[i]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix());

		//Creating the Mesh objects
		// Set buffers in the input assembler
		//  - Do this ONCE PER OBJECT you're drawing, since each object might
		//    have different geometry.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11Buffer* vertexBufferPtr = envObjects[i]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
		context->IASetIndexBuffer(envObjects[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		// Finally do the actual drawing
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		context->DrawIndexed(
			envObjects[i]->GetMesh()->GetIndexCount(),   // The number of indices to use (we could draw a subset if we wanted)
			0,							// Offset to the first index we want to use
			0);
	}

	for (uint16_t i = 0; i < planetObjects.size(); i++)
	{
		//Not drawing transparent objects first
		{
			planetObjects[i]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix());

			//Creating the Mesh objects
			// Set buffers in the input assembler
			//  - Do this ONCE PER OBJECT you're drawing, since each object might
			//    have different geometry.
			UINT stride = sizeof(Vertex);
			UINT offset = 0;

			ID3D11Buffer* vertexBufferPtr = planetObjects[i]->GetMesh()->GetVertexBuffer();
			context->IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
			context->IASetIndexBuffer(planetObjects[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

			// Finally do the actual drawing
			//  - Do this ONCE PER OBJECT you intend to draw
			//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
			//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
			//     vertices in the currently set VERTEX BUFFER
			context->DrawIndexed(
				planetObjects[i]->GetMesh()->GetIndexCount(),   // The number of indices to use (we could draw a subset if we wanted)
				0,							// Offset to the first index we want to use
				0);
		}
	}

	// After I draw any and all opaque entities, I want to draw the sky
	ID3D11Buffer* skyVB = meshObjects[7]->GetVertexBuffer();
	ID3D11Buffer* skyIB = meshObjects[7]->GetIndexBuffer();

	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	// Set up the sky shaders
	skyVS->SetMatrix4x4("view", camera->getViewMatrix());
	skyVS->SetMatrix4x4("projection", camera->getProjectionMatrix());
	skyVS->CopyAllBufferData();
	skyVS->SetShader();

	skyPS->SetShaderResourceView("SkyTexture", skySRV);
	skyPS->SetSamplerState("BasicSampler", sampler3);
	skyPS->SetShader();

	// Set up the render states necessary for the sky
	context->RSSetState(skyRastState);
	context->OMSetDepthStencilState(skyDepthState, 0);
	context->DrawIndexed(meshObjects[7]->GetIndexCount(), 0, 0);

	// When done rendering, reset any and all states for the next frame
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

	//Transparent objects
	context->OMSetBlendState(blendState, 0, 0xFFFFFFFF);
	float alpha = 1.0f;
	if (plankBeingPlaced)
	{
		//last object in gameObjects
		size_t i = gameObjects.size() - 1;
		alpha = 1 - ((gameObjects[i]->GetPosition().y - finalPositionOfLatestPlankCreated.y) / 2);
		//gameObjects[i]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix(), alpha);
		if (gameObjects[i]->GetScale().x > 1.0f)
		{
			gameObjects[i]->PrepareMaterialWater(camera->getViewMatrix(), camera->getProjectionMatrix(), time, 1, alpha);
		}
		else
		{
			gameObjects[i]->PrepareMaterialWater(camera->getViewMatrix(), camera->getProjectionMatrix(), time, 0, alpha);
		}
		//Creating the Mesh objects
		// Set buffers in the input assembler
		//  - Do this ONCE PER OBJECT you're drawing, since each object might
		//    have different geometry.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11Buffer* vertexBufferPtr = gameObjects[i]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
		context->IASetIndexBuffer(gameObjects[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		// Finally do the actual drawing
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		context->DrawIndexed(
			gameObjects[i]->GetMesh()->GetIndexCount(),   // The number of indices to use (we could draw a subset if we wanted)
			0,							// Offset to the first index we want to use
			0);
	}

	if (plankBeingRemoved)
	{
		//Oldest object apart from the ball at 0, so 1
		alpha = (gameObjects[1]->GetPosition().y - finalPositionOfDeletingPlank.y) / 2;
		//gameObjects[1]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix(), alpha);

		if (gameObjects[1]->GetScale().x > 1.0f)
		{
			gameObjects[1]->PrepareMaterialWater(camera->getViewMatrix(), camera->getProjectionMatrix(), time, 1, alpha);
		}
		else
		{
			gameObjects[1]->PrepareMaterialWater(camera->getViewMatrix(), camera->getProjectionMatrix(), time, 0, alpha);
		}

		//Creating the Mesh objects
		// Set buffers in the input assembler
		//  - Do this ONCE PER OBJECT you're drawing, since each object might
		//    have different geometry.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11Buffer* vertexBufferPtr = gameObjects[1]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
		context->IASetIndexBuffer(gameObjects[1]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		// Finally do the actual drawing
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		context->DrawIndexed(
			gameObjects[1]->GetMesh()->GetIndexCount(),   // The number of indices to use (we could draw a subset if we wanted)
			0,							// Offset to the first index we want to use
			0);
	}

	//Particle states add timer after 
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);  // Additive blending
	context->OMSetDepthStencilState(particleDepthState, 0);			// No depth WRITING

	if (!isFalling)
	{
		// Draw the emitter
		emitter->Draw(context, camera);
	}

	//Reset blendstate
	context->OMSetBlendState(0, blend, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);

	//Blur
	if (currentGameMode != inGame)
	{
		// Reset any states we've changed for the next frame!
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);

		// After we're done rendering the ENTIRE scene
		// (opaque, transparent, sky, particles, etc.)
		// Swap back to the back buffer and draw using our
		// specialized post process shaders
		context->OMSetRenderTargets(1, &backBufferRTV, 0);

		// Prepare post process shaders 
		postProcessingVS->SetShader();

		postProcessingPS->SetShader();
		postProcessingPS->SetShaderResourceView("Pixels", postProcessingSRV);
		postProcessingPS->SetSamplerState("Sampler", sampler1);
		postProcessingPS->SetInt("blurAmount", 6);
		postProcessingPS->SetFloat("pixelWidth", 1.0f / width);
		postProcessingPS->SetFloat("pixelHeight", 1.0f / height);
		postProcessingPS->CopyAllBufferData();

		// Turn off my geometry buffers
		ID3D11Buffer* switchOff = 0;
		context->IASetVertexBuffers(0, 1, &switchOff, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		// Actuall draw the "full screen triangle"
		context->Draw(3, 0);

		// Unbind the shader resource view so there is not 
		// resource fighting (contention) at the start of the next frame
		postProcessingPS->SetShaderResourceView("Pixels", 0);
	}

	//SpriteBatch begin
	if (currentGameMode == start)
	{
		spriteBatch->Begin();
		spriteBatch->Draw(SRVVariableStartDisplay, startRect, Colors::White*0.6f);
		spriteBatch->Draw(SRVTitle, titleRect, Colors::White);
		spriteBatch->End();

		//Reset blendstate again
		context->OMSetBlendState(0, blend, 0xffffffff);
		context->OMSetDepthStencilState(0, 0);
	}
	else if (currentGameMode == pause)
	{
		spriteBatch->Begin();
		spriteBatch->Draw(SRVPaused, pauseRect, Colors::White*0.6f);
		spriteBatch->End();

		//Reset blendstate again
		context->OMSetBlendState(0, blend, 0xffffffff);
		context->OMSetDepthStencilState(0, 0);
	}
	else if (currentGameMode == gameOver)
	{
		if (gameOverCreditsTimer > 1.0f)
		{
			spriteBatch->Begin();
			float alphaForGameOver = (gameOverCreditsTimer - 1.0f) / 2.0f;
			if (alphaForGameOver >= 3.0f)
			{
				alphaForGameOver = 1.0f;
			}
			spriteBatch->Draw(SRVGameOver, gameOverRect, Colors::White*alphaForGameOver);
			if (gameOverCreditsTimer > 2.5f)
			{
				float alphaForCredits = (gameOverCreditsTimer - 2.5f);
				if (alphaForCredits >= 3.5f)
				{
					alphaForCredits = 1.0f;
				}
				spriteBatch->Draw(SRVCredits, creditsRect, Colors::White*alphaForCredits);
			}

			if (gameOverCreditsTimer > 1.0f)
			{
				spriteBatch->Draw(SRVEscape, escRect, Colors::White*alphaForEsc);
				spriteBatch->End();
			}

			//Reset blendstate again
			context->OMSetBlendState(0, blend, 0xffffffff);
			context->OMSetDepthStencilState(0, 0);
		}
	}

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

void Game::LoadTheDirectionalLight()
{
	sun.AmbientColor = XMFLOAT4{ 0.1f,0.1f,0.1f,1.0f };
	sun.DiffuseColor = XMFLOAT4{ 1.0f,1.0f,1.0f,1.0f };
	sun.Direction = XMFLOAT3{ 1.0f, -0.0f, 0.0f };

	sun2.AmbientColor = XMFLOAT4{ 0.1f,0.1f,0.1f,1.0f };
	sun2.DiffuseColor = XMFLOAT4{ 0.6f, 0.6f, 0.9f, 1.0f };
	sun2.Direction = XMFLOAT3{ 0.0f, -4.0f, 0.0f };
}

void Game::InitialisingLocalVariables()
{
	pathPosition = XMFLOAT3(0.0f, 1.52f, 2.0f);
	lastStraightCreated = true;
	isFalling = false;
	LoadTheDirectionalLight();
	LoadShadersAndTextures();
	CreateCamera();
	CreateBasicGeometry();
	gravity = -0.8f;
	currentEmitterColor = water;

	ID3D11Texture2D* postProcessingTexture;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&textureDesc, 0, &postProcessingTexture);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(postProcessingTexture, &rtvDesc, &postProcessingRenderTarget);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(postProcessingTexture, &srvDesc, &postProcessingSRV);
	postProcessingTexture->Release();
}

/*----------------------------------*/

void Game::SpawnEnvObjects()
{
	//JASON - Random Position
	XMFLOAT3 ballPosition = gameObjects[0]->GetPosition();
	XMFLOAT3 position = ballPosition;
	position.x -= ((rand() % 6) + 15);
	//position.y += ((rand() % 6) + 15);
	position.z += ((rand() % 6) + 15);
	
	position.y += ((rand() % 13) + 8)/10.0f;
	if (rand() % 2 == 1)
	{
		position.y = -5;
	}
	else
	{
		position.y = 2;
	}

	//JASON - Random Rotation
	int rotationValue = ((rand() % 5));
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	if (rotationValue == 1)
	{
		rotation = XMFLOAT3(degreeRotation, degreeRotation, degreeRotation);
	}
	else if (rotationValue == 2)
	{
		rotation = XMFLOAT3((degreeRotation*2), (degreeRotation*2),(degreeRotation*2));
	}
	else if (rotationValue == 3)
	{
		rotation = XMFLOAT3((degreeRotation*3), (degreeRotation*3),(degreeRotation*3));
	}
	else if (rotationValue == 4)
	{
		rotation = XMFLOAT3((degreeRotation/2),(degreeRotation/2), (degreeRotation/2));
	}
	else if (rotationValue == 0)
	{
		//DO nothing
	}

	//JASON - Random Scale - 
	float scaleValue = ((rand() % 16) /1000.0f) + .05f;
	XMFLOAT3 scale = XMFLOAT3(scaleValue, scaleValue, scaleValue);

	GameEntity *envObject1 = new GameEntity(position, rotation, scale, asteroid, envMaterials[0]);
	envObjects.push_back(envObject1);
}

void Game::SpawnVenus()
{
	//Position Rotation Scale
	//XMFLOAT3 positionVenus = XMFLOAT3(-40.0f, -10.0f, 80.0f);

	XMFLOAT3 ballPosition = gameObjects[0]->GetPosition();
	XMFLOAT3 position = ballPosition;
	position.x -= ((rand() % 50));
	//position.y += ((rand() % 6) + 15);
	position.z += ((rand() % 50) + 15);

	position.y += ((rand() % 13) + 8) / 10.0f;
	if (rand() % 2 == 1)
	{
		position.y = -15;
	}
	else
	{
		position.y = 15;
	}

	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(2.0f, 2.0f, 2.0f);
	GameEntity *planetObject2 = new GameEntity(position, rotation, scale, venus, planetMaterials[rand() % (planetMaterials.size())]);
	planetObjects.push_back(planetObject2);
}

void Game::SpawnTimerPlanets(float deltaTime) //JASON - Timer for spawning objects
{
	timer1 += deltaTime;
	{
		if (timer1 > 10.5f)
		{
			SpawnVenus();
			timer1 = 0;
		}
		if (planetObjects.size() > 5)
		{
			delete planetObjects[0];
			planetObjects.erase(planetObjects.begin());
		}
	}
}

void Game::SpawnTimer(float deltaTime) //JASON - Timer for spawning objects
{
	timer += deltaTime;
	{
		if (timer > 2.5f)
		{
			SpawnEnvObjects();
			timer = 0;
		}
		if (envObjects.size() > 25)
		{
			delete envObjects[0];
			envObjects.erase(envObjects.begin());
		}
	}
}


/*----------------------------------*/


void Game::CreatePath()
{
	if (rand() % 2 == 1)
	{
		CreatePlankStraight(materialObjects[(rand() % 3) + 2]);
	}
	else
	{
		CreatePlankLeft(materialObjects[(rand() % 3) + 2]);
	}
}

void Game::CreatePlankStraight(Material* material)
{
	//Common positions for now
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 0.2f, 5.0f);

	if (!lastStraightCreated)
	{
		pathPosition.x += 2.0f;
		pathPosition.z += 2.0f;
	}
	finalPositionOfLatestPlankCreated = pathPosition;
	finalPositionOfLatestPlankCreated.y -= 2.0f;
	GameEntity *plankStraight = new GameEntity(pathPosition, rotation, scale, meshObjects[7], material);
	pathPosition.z += 5.0f;

	gameObjects.push_back(plankStraight);
	CheckIfNeedToRemovePlanks();
	lastStraightCreated = true;
	plankBeingPlaced = true;
}

void Game::CreatePlankLeft(Material* material)
{
	//Common positions for now
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(5.0f, 0.2f, 1.0f);
	if (lastStraightCreated)
	{
		pathPosition.x -= 2.0f;
		pathPosition.z -= 2.0f;
	}
	finalPositionOfLatestPlankCreated = pathPosition;
	finalPositionOfLatestPlankCreated.y -= 2.0f;
	GameEntity *plankLeft = new GameEntity(pathPosition, rotation, scale, meshObjects[7], material);
	pathPosition.x -= 5.0f;

	gameObjects.push_back(plankLeft);
	CheckIfNeedToRemovePlanks();
	lastStraightCreated = false;
	plankBeingPlaced = true;
}

void Game::CheckIfNeedToRemovePlanks()
{
	if (!plankBeingRemoved && gameObjects.size() > 8)
	{
		plankBeingRemoved = true;
		finalPositionOfDeletingPlank = gameObjects[1]->GetPosition();
		finalPositionOfDeletingPlank.y -= 1.0f;
	}
}

void Game::EnableBlending()
{
	// Fill out a description and create the state
	D3D11_RASTERIZER_DESC rd = {};
	rd.CullMode = D3D11_CULL_NONE;
	rd.FillMode = D3D11_FILL_SOLID;

	device->CreateRasterizerState(&rd, &rsState);
	// Set up a blend state
	D3D11_BLEND_DESC bd = {};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	bd.RenderTarget[0].BlendEnable = true;

	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&bd, &blendState);
}

void Game::MoveBallOnPlatform(float deltaTime)
{
	//Ball rotation
	gameObjects[0]->RotateRelative(10.0f * deltaTime, 0.0f, 0.0f);

	//Move platform depending on the ball's rotation
	if (isBallDirectionLeft)
	{
		gameObjects[0]->MoveRelative(0.0f, 0.0f, +2.5f * deltaTime);
		//camera->MoveRelative(0.0f, 0.0f, +2.5f * deltaTime);
	}
	else
	{
		gameObjects[0]->MoveRelative(-2.5f * deltaTime, 0.0f, 0.0f);
		//camera->MoveRelative(-2.5f * deltaTime, 0.0f, 0.0f);
	}

	if (isFalling)
	{
		gameObjects[0]->Falling(deltaTime, gravity);
	}
}

void Game::CheckPhysics()
{
	if (!isFalling)
	{
		XMFLOAT3 plankSize;
		XMFLOAT3 plankPosition;
		XMFLOAT3 ballPosition;
		ballPosition = gameObjects[0]->GetPosition();
		for (int i = 1; i < gameObjects.size(); i++)
		{
			plankSize = gameObjects[i]->GetScale();
			plankPosition = gameObjects[i]->GetPosition();
			if ((ballPosition.x < plankPosition.x + 0.7*plankSize.x) && (ballPosition.x > plankPosition.x - 0.7*plankSize.x) &&
				(ballPosition.z < plankPosition.z + 0.7*plankSize.z) && (ballPosition.z > plankPosition.z - 0.7*plankSize.z))
			{
				if (i >= gameObjects.size() - 2)
				{
					timeToCreate = true;
				}

				EmitterColor theMainMaterial = gameObjects[i]->GetMaterial()->GetColor();
				if (currentEmitterColor != theMainMaterial)
				{
					currentEmitterColor = theMainMaterial;
					emitter->ChangeColor(currentEmitterColor);
				}
				//i-th position is the plank the ball is above. Thats the change for emitter settings
				//Emitter is progress
				return;
			}
		}
		isFalling = true;
		camera->SetGameMode(gameOver);
		currentGameMode = gameOver;
	}
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (currentGameMode == start && SRVVariableStartDisplay == SRVStart1)
	{
		currentGameMode = inGame;
		camera->SetGameMode(inGame);
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;


	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	//RECT titleRect = { (width / 2) - 120, (height / 2) + 120, (width / 2) + 100 , (height / 2) + 160 };
	height1ToCheck = ((height / 2) + (height / 6));
	height2ToCheck = ((height / 2) + (height / 4));
	width1ToCheck = ((width / 2) - (width / 11));
	width2ToCheck = ((width / 2) + (width / 13));
	if (currentGameMode == start)
	{
		if ((unsigned int)x > width1ToCheck && (unsigned int)x < width2ToCheck &&
			(unsigned int)y >height1ToCheck && (unsigned int)y < height2ToCheck)
		{
			SRVVariableStartDisplay = SRVStart1;
		}
		else
		{
			SRVVariableStartDisplay = SRVStart2;
		}
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion