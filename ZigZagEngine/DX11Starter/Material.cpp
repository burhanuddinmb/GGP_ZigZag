#include "Material.h"

Material::Material(SimpleVertexShader * vertexShader, SimplePixelShader * pixelShader, ID3D11ShaderResourceView* SRV, ID3D11SamplerState* sampler)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->SRV = SRV;
	this->sampler = sampler;
}

Material::~Material()
{
	SRV->Release();
	sampler->Release();
	delete vertexShader;
	delete pixelShader;
}

SimpleVertexShader * Material::GetVertexShader()
{
	return vertexShader;
}

SimplePixelShader * Material::GetPixelShader()
{
	return pixelShader;
}

ID3D11ShaderResourceView * Material::GetSRV()
{
	return SRV;
}

ID3D11SamplerState * Material::GetSampler()
{
	return sampler;
}
