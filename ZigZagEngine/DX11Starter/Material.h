#pragma once
#include "SimpleShader.h"

class Material
{
public:
	Material(SimpleVertexShader *vertexShader, SimplePixelShader *pixelShader, ID3D11ShaderResourceView* SRV, ID3D11SamplerState* sampler);
	~Material();

	SimpleVertexShader *GetVertexShader();
	SimplePixelShader *GetPixelShader();
	ID3D11ShaderResourceView* GetSRV();
	ID3D11SamplerState* GetSampler();

private:
	ID3D11ShaderResourceView* SRV;
	ID3D11SamplerState* sampler;
	SimpleVertexShader *vertexShader;
	SimplePixelShader *pixelShader;
};

