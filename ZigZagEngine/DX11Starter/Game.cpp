#include "Game.h"
#include "Vertex.h"
#include <d3d11.h>
#include <conio.h>

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

	if (rsState != nullptr) rsState->Release();

	if (blendState != nullptr) blendState->Release();

	delete camera;

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

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
	CreateWICTextureFromFile(device, context, L"../../Assets/Materials/waves.jpeg", 0, &SRV2);

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

	//Eventually put everything in the material
	// *** NOTICE ***
	//The variables that we are passing, get deleted in material
	//We shall change implementation if needed
	materialObjects.push_back(new Material(vertexShader1, pixelShader1, SRV1, sampler1));
	materialObjects.push_back(new Material(vertexShader2, pixelShader2, SRV2, sampler2));
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

	Vertex triangleVertices[] = 
	{
		{ XMFLOAT3(-1.0f, +0.0f, +0.0f), normal, uv },
		{ XMFLOAT3(+0.0f, +1.0f, +0.0f), normal, uv },
		{ XMFLOAT3(+1.0f, +0.0f, +0.0f), normal, uv },
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
		{ XMFLOAT3(-1.0f, -1.0f, +0.0f), normal, uv },
		{ XMFLOAT3(-1.0f, +1.0f, +0.0f), normal, uv },
		{ XMFLOAT3(+1.0f, +1.0f, +0.0f), normal, uv },
		{ XMFLOAT3(+1.0f, -1.0f, +0.0f), normal, uv },
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
		{ XMFLOAT3(+1.2f, -0.8f, +0.0f), normal, uv },
		{ XMFLOAT3(+1.2f, +0.8f, +0.0f), normal, uv },
		{ XMFLOAT3(+2.2f, +1.2f, +0.0f), normal, uv },
		{ XMFLOAT3(+3.0f, +0.0f, +0.0f), normal, uv },
		{ XMFLOAT3(+2.2f, -1.2f, +0.0f), normal, uv },
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
		{ XMFLOAT3(-3.4f, -1.5f, +0.0f), normal, uv },
		{ XMFLOAT3(-3.4f, +1.5f, +0.0f), normal, uv },
		{ XMFLOAT3(-1.1f, +1.5f, +0.0f), normal, uv },
		{ XMFLOAT3(-1.1f, +0.0f, +0.0f), normal, uv },
		{ XMFLOAT3(-1.5f, -0.5f, +0.0f), normal, uv },
		{ XMFLOAT3(-1.9f, -0.0f, +0.0f), normal, uv },
		{ XMFLOAT3(-1.9f, +0.2f, +0.0f), normal, uv },
		{ XMFLOAT3(-2.6f, +0.2f, +0.0f), normal, uv },
		{ XMFLOAT3(-2.6f, -1.5f, +0.0f), normal, uv },
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

	//gameObjects.push_back(entity1);
	//gameObjects.push_back(entity2);
	//gameObjects.push_back(entity3);
	//gameObjects.push_back(entity4);
	gameObjects.push_back(entity5);

	//Put the two planks
	CreatePlankStraight();
	CreatePlankStraight();

	//Correct position
	XMFLOAT3 tmpPosition;
	for (int i = 1; i < gameObjects.size(); i++)
	{
		tmpPosition = gameObjects[i]->GetPosition();
		tmpPosition.y -= 2.0f;
		gameObjects[i]->SetPosition(tmpPosition);
	}
	plankBeingPlaced = false;
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

	MoveBallOnPlatform(deltaTime);
	CheckPhysics();
	if ((int)totalTime == timeToCheck)
	{
		timeToCheck += 2;
		CreatePath();
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
	const int KEY_UP = 0x1;
	if (!isFalling && (GetAsyncKeyState(VK_SPACE) & KEY_UP) == KEY_UP)
	{

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

	//camera->Update(deltaTime);
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
	const float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);


	//Draw order shadowMap, objects, transparent objects

	//Drawing objects
	for (uint16_t i = 0; i < gameObjects.size(); i++)
	{
		//Not drawing transparent objects first
		if ((plankBeingPlaced && (i == (gameObjects.size() - 1) )) || (plankBeingRemoved && i == 1))
		{
			continue;
		}

		gameObjects[i]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix());

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

	//Transparent objects
	float alpha = 1.0f;
	if (plankBeingPlaced)
	{
		//last object in gameObjects
		int i = gameObjects.size() - 1;
		alpha = 1 - ((gameObjects[i]->GetPosition().y - finalPositionOfLatestPlankCreated.y)/2);
		gameObjects[i]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix(), alpha);

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
		alpha = (gameObjects[1]->GetPosition().y - finalPositionOfDeletingPlank.y)/2;
		gameObjects[1]->PrepareMaterial(camera->getViewMatrix(), camera->getProjectionMatrix(), alpha);

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
	sun2.DiffuseColor = XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f };
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
	gravity = 0.004f;
}

void Game::CreatePath()
{
	if (rand() % 2 == 1)
	{
		CreatePlankStraight();
	}
	else
	{
		CreatePlankLeft();
	}
}

void Game::CreatePlankStraight()
{
	//Common positions for now
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 0.1f, 5.0f);

	if (!lastStraightCreated)
	{
		pathPosition.x += 2.0f;
		pathPosition.z += 2.0f;
	}
	finalPositionOfLatestPlankCreated = pathPosition;
	finalPositionOfLatestPlankCreated.y -= 2.0f;
	GameEntity *plankStraight = new GameEntity(pathPosition, rotation, scale, meshObjects[7], materialObjects[0]);
	pathPosition.z += 5.0f;

	gameObjects.push_back(plankStraight);
	CheckIfNeedToRemovePlanks();
	lastStraightCreated = true;
	plankBeingPlaced = true;
}

void Game::CreatePlankLeft()
{
	//Common positions for now
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(5.0f, 0.1f, 1.0f);
	if (lastStraightCreated)
	{
		pathPosition.x -= 2.0f;
		pathPosition.z -= 2.0f;
	}
	finalPositionOfLatestPlankCreated = pathPosition;
	finalPositionOfLatestPlankCreated.y -= 2.0f;
	GameEntity *plankLeft = new GameEntity(pathPosition, rotation, scale, meshObjects[7], materialObjects[0]);
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

	context->OMSetBlendState(blendState, 0, 0xFFFFFFFF);
}

void Game::MoveBallOnPlatform(float deltaTime)
{
	//Ball rotation
	gameObjects[0]->RotateRelative(10.0f * deltaTime, 0.0f, 0.0f);

	//Move platform depending on the ball's rotation
	if (isBallDirectionLeft)
	{
		gameObjects[0]->MoveRelative(0.0f, 0.0f, +2.5f * deltaTime);
		camera->MoveRelative(0.0f, 0.0f, +2.5f * deltaTime);
	}
	else
	{
		gameObjects[0]->MoveRelative(-2.5f * deltaTime, 0.0f, 0.0f);
		camera->MoveRelative(-2.5f * deltaTime, 0.0f, 0.0f);
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
				return;
			}
		}
		isFalling = true;
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
	if (buttonState & 0x0002) {
		camera->MoveByMouse((float)(x - prevMousePos.x)*0.01f, (float)(y - prevMousePos.y)*0.01f);
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