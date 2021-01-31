#define MaxFinalTransform 80

float4x4 WorldViewProjection;

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

    

    return Output;
};