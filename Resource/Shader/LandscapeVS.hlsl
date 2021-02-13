float4x4 World;
float4x4 View;
float4x4 Projection;

struct Input
{
    float4 Location : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct Output
{
    float4 Location : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

Output main(Input In)
{
    float4x4 WorldView= mul(World, View);
    float4x4 WorldViewProjection = mul(WorldView, Projection);
    
    Output Out;
    
    Out.Location = mul(float4(In.Location.xyz, 1.f), WorldViewProjection);
    Out.Normal = mul(float4(In.Normal, 0.f), WorldViewProjection).xyz; 
    Out.UV = In.UV;
    
    return Out;
}