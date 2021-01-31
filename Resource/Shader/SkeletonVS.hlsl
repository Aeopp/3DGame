#define MaxFinalTransform 61

float4x4 World;
float4x4 ViewProjection;
float4x4 FinalMatrix[MaxFinalTransform];

struct VS_INPUT
{
    float4 Location : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
    float4 BoneIds : TEXCOORD1;
    float4 BoneWeights : TEXCOORD2;
};

struct VS_OUTPUT
{
    float4 Location : POSITION;
    float3 Normal : TEXCOORD0;
    float2 UV : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT Input)
{
    VS_OUTPUT Output = (VS_OUTPUT)0;
	Input.Location.w = 1.0f;
	float4 FinalLocation = float4(0, 0, 0, 1);
    
	FinalLocation.xyz += (mul(Input.Location, FinalMatrix[Input.BoneIds.x]) * Input.BoneWeights.x).xyz;
	FinalLocation.xyz += (mul(Input.Location, FinalMatrix[Input.BoneIds.y]) * Input.BoneWeights.y).xyz;
	FinalLocation.xyz += (mul(Input.Location, FinalMatrix[Input.BoneIds.z]) * Input.BoneWeights.z).xyz;
	FinalLocation.xyz += (mul(Input.Location, FinalMatrix[Input.BoneIds.w]) * Input.BoneWeights.w).xyz;
	FinalLocation.w = 1.0f;
    
    float4x4 WorldViewProjection = World * ViewProjection;
    
	FinalLocation = mul(FinalLocation, WorldViewProjection);
	Output.Location = FinalLocation;
//    Output.Location = mul(Input.Location, WorldViewProjection);
    Output.Normal = mul(Input.Normal, (float3x3)WorldViewProjection);
	Output.UV = Input.UV;
    
    return Output;
};