#pragma once
#include "SimpleShader.h"
#include "Emitter.h"
class Material
{
public:
	Material(SimpleVertexShader *vertexShader, SimplePixelShader *pixelShader, ID3D11ShaderResourceView* SRV, ID3D11SamplerState* sampler, EmitterColor colorName = other);
	~Material();

	SimpleVertexShader *GetVertexShader();
	SimplePixelShader *GetPixelShader();
	ID3D11ShaderResourceView* GetSRV();
	ID3D11SamplerState* GetSampler();
	EmitterColor GetColor();

private:
	ID3D11ShaderResourceView* SRV;
	ID3D11SamplerState* sampler;
	SimpleVertexShader *vertexShader;
	SimplePixelShader *pixelShader;
	EmitterColor colorName;
};

