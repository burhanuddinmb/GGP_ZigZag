
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};


cbuffer externalData : register(b0)
{
	DirectionalLight sun;
	DirectionalLight sun2;
};

Texture2D diffuseTexture  : register(t0);
SamplerState basicSampler : register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering

	//Need to implement an if-else code to add different effect on different things

	//Normal
	input.normal = normalize(input.normal);

	//Texture
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);

	//Sun1
	float3 normalToSun1 = normalize(-sun.Direction);
	float surfaceDotSun1 = saturate(dot(input.normal, normalToSun1));
	float4 sun1Light = (sun.DiffuseColor*surfaceDotSun1) + sun.AmbientColor;

	//Sun2
	float3 normalToSun2 = normalize(-sun2.Direction);
	float surfaceDotSun2 = saturate(dot(input.normal, normalToSun2));
	float4 sun2Light = (sun2.DiffuseColor*surfaceDotSun2) + sun2.AmbientColor;

	//Light multiplied with texture
	return surfaceColor*(sun1Light + sun2Light);
}