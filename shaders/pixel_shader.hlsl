
Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
TextureCube texCube : register(t2); //New

struct PSIn
{
	float4 Pos  : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEX;
	float3 WorldPos : Pos;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
};

SamplerState texSampler : register (s0);
SamplerState cubeSampler : register (s1); //New

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

}

//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	//New
	float3 N = normalize( input.Normal );
	float3 B = normalize( input.Binormal);
	float3 T = normalize( input.Tangent);
	//return float4(input.Normal * 0.5 + 0.5, 1); //For debugging with colors

	float3x3 TBN = transpose(float3x3 (T, B, N));
	//Construct new normal with the TBN matrix
	float4 NormalVector = texNormal.Sample(texSampler, input.TexCoord) * 2 - 1; //From color to vector (decode) 
	float3 N_ = mul(TBN, NormalVector.xyz);
	float3 LookUpVector = normalize(input.WorldPos - CamPosition.xyz);
	float3 ViewVector = normalize(CamPosition.xyz - input.WorldPos);

	//Replace input.Normal with the new normal TBN * N
	float3 LightVector1 = normalize(LightPosition1.xyz - input.WorldPos);
	float3 ReflectVector1 = normalize(reflect(-LightVector1.xyz, N_));

	/*float3 LightVector2 = normalize(LightPosition2 - input.worldPos);
	float3 ReflectVector2 = normalize(reflect(LightVector2, input.Normal));*/
	/*float3 LightSum = (mul(Diffuse.xyz, LightVector1 * input.Normal) + mul(Specular.xyz, pow(ReflectVector1 * ViewVector, 1)))
		+ (mul(Diffuse.xyz, LightVector2 * input.Normal) + mul(Specular.xyz, pow(ReflectVector2 * ViewVector, 1)));*/

	/*float4 CubeColor = texCube.Sample(cubeSampler, LookUpVector);
	return CubeColor;*/ //For skybox part 2
	
	float3 R = normalize(reflect(LookUpVector, N_));
	float4 CubeColor = texCube.Sample(cubeSampler, R);
	//return CubeColor; //Perfect mirror
	float Shinyness = Specular.w;
	float4 DiffuseColor = texDiffuse.Sample(texSampler, input.TexCoord);	

	float3 A = Ambient.xyz * CubeColor.xyz; //DiffuseColor.xyz * 0.3f;
	float3 D = mul(DiffuseColor.xyz * CubeColor.xyz, max(dot(LightVector1.xyz, N_), 0)); //Diffuse.xyz
	//float3 DR = mul(D, CubeColor.xyz);
	float3 S = mul(Specular.xyz, pow(max(dot(ReflectVector1, ViewVector), 0), Shinyness));
	float3 SR = mul(S, CubeColor.xyz);
	
	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1
	//return float4(input.Normal*0.5+0.5, 1);

	return float4(A * 0.1f + D + SR, 1);
	//return float4(input.Color, 1);
	//return texDiffuse.Sample(texSampler, input.TexCoord); //L3
	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
	//return float4(input.TexCoord, 0, 1);
}