
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
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
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
	float time;
	float alpha;
	int scrollNumber;  // 0 = x   and    1 = y
};

Texture2D diffuseTexture  : register(t0);
Texture2D WaterNormal  : register(t1);
Texture2D WaterNormal1  : register(t2);
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

//Tangent
input.tangent = normalize(input.tangent);

float2 normalMap1UV = input.uv;

//uv scrolling

if(scrollNumber==0)
input.uv.x = input.uv.x + time*0.6;
else
input.uv.y = input.uv.y - time*0.6;
//normalMap1UV.x = normalMap1UV.x + time;


//1st Normal Map
// Sample the normal map
float3 normalFromMap = WaterNormal.Sample(basicSampler, input.uv).rgb;

// Make sure to unpack the normal
normalFromMap = normalize(normalFromMap * 2 - 1);

// We need the various component vectors for tangent-to-world space
float3 N = input.normal;
float3 T = normalize(input.tangent - N * dot(input.tangent, N));
float3 B = cross(T, N);

// Create the TBN matrix which we use to convert from tangent to world space
float3x3 TBN = float3x3(T, B, N);
input.normal = normalize(mul(normalFromMap, TBN));

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
float4 outputColor = surfaceColor * (sun1Light + sun2Light);
outputColor.a = alpha;
//Light multiplied with texture
//return float4(alpha, alpha, alpha, alpha);
return outputColor;
}