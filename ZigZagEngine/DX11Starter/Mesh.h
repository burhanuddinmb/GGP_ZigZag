#pragma once
#include <d3d11.h>
#include "Vertex.h"
#include <Windows.h>

class Mesh
{
public:
	//Initiates the vertex Buffer and indexBuffer
	Mesh(Vertex *vertices, int noOfVertices, int *indices, int noOfIndices, ID3D11Device *device);
	Mesh(const char* objFile, ID3D11Device* device);
	~Mesh();

	//Returns the vertexBuffer pointer
	ID3D11Buffer* GetVertexBuffer();

	//Returns the vertexBuffer pointer
	ID3D11Buffer* GetIndexBuffer();

	//Returns the indices the object contains
	int GetIndexCount();

	

private:

	void InitializeData(Vertex *vertices, int noOfVertices, int *indices, int noOfIndices, ID3D11Device *device);

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	uint16_t noOfIndices;
};

