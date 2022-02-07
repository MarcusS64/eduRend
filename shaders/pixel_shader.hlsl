
Texture2D texDiffuse : register(t0);

struct PSIn
{
	float4 Pos  : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEX;
	float3 WorldPos : Pos;
};

SamplerState cubeSampler : register (s1);

cbuffer LightCamBuffer : register(b0)
{
	float4 CamPosition;
	float4 LightPosition1;
}

cbuffer PhongShinyBuffer : register(b1)
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	//float4 shinyness;
}

//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	float3 LightVector1 = normalize(LightPosition1.xyz - input.WorldPos);
	float3 ReflectVector1 = normalize(reflect(LightVector1.xyz, input.Normal));

	/*float3 LightVector2 = normalize(LightPosition2 - input.worldPos);
	float3 ReflectVector2 = normalize(reflect(LightVector2, input.Normal));*/

	float3 ViewVector = normalize(CamPosition.xyz - input.WorldPos);

	/*float3 LightSum = (mul(Diffuse.xyz, LightVector1 * input.Normal) + mul(Specular.xyz, pow(ReflectVector1 * ViewVector, 1)))
		+ (mul(Diffuse.xyz, LightVector2 * input.Normal) + mul(Specular.xyz, pow(ReflectVector2 * ViewVector, 1)));*/

	float Shinyness = Specular.w;

	float3 A = Ambient.xyz;
	float3 D = mul(Diffuse.xyz, max(dot(LightVector1.xyz, input.Normal), 0));
	float3 S = mul(Specular.xyz, pow(max(dot(ReflectVector1, ViewVector), 0), Shinyness));
	//float3 DiffuseColor = mul(DiffuseTexColor.xyz, dot(LightVector1, input.Normal));
	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1
	//return float4(input.Normal*0.5+0.5, 1);

	return float4(A + D + S, 1);
	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
	//return float4(input.TexCoord, 0, 1);
}