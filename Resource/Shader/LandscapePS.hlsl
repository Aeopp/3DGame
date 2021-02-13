



sampler2D Diffuse;


struct Input
{
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

float4 main(Input In) : COLOR0
{
    float4 Color = tex2D(Diffuse, In.UV);
    
    return Color;
}