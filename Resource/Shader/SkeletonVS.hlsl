matrix World;
matrix View;
matrix Projection;
float4 WorldCameraLocation;
int UVFlag;

int bUI;

struct VS_INPUT
{
    float4 Location : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
    float2 UV : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Location : POSITION;
    float2 UV : TEXCOORD0;
    float3 ViewDirection : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3 Tangent : TEXCOORD3;
    float3 BiNormal : TEXCOORD4;
    float3 WorldLocation : TEXCOORD5;
    float ViewZ : TEXCOORD6;
};

VS_OUTPUT main(VS_INPUT Input)
{
    VS_OUTPUT Output = (VS_OUTPUT)0;

    if (bUI)
    {
        Output.Location = mul(Input.Location, World);
        Output.Location = mul(Output.Location, View);
        Output.Location = mul(Output.Location, Projection);
        Output.UV = Input.UV;

        return Output;
    }


    // Vertex Location : Local Coord -> World Coord 
    Output.Location = mul(Input.Location, World);
    Output.WorldLocation = Output.Location;

    // WorldViewDirection Calc
    float3 ViewDirection = normalize(Output.Location.xyz - WorldCameraLocation.xyz);
    Output.ViewDirection = ViewDirection;

    // Vertex Location : World Coord -> View Coord ... -> Projection Coord
    Output.Location = mul(Output.Location, View);
    Output.ViewZ = Output.Location.z;
    Output.Location = mul(Output.Location, Projection);

    // Vertex Normal -> Local Coord -> World Coord
    float3 WorldNormal = mul(Input.Normal, (float3x3) World);
    Output.Normal = normalize(WorldNormal);

    Output.UV = Input.UV;
    // Y Inverse
    if (UVFlag == 2)
    {
        Output.UV.y *= -1.f;
    };
    float3 WorldTangent = mul(Input.Tangent, (float3x3) World);
    Output.Tangent = normalize(WorldTangent);
    float3 WorldBiNormal = mul(Input.BiNormal, (float3x3) World);
    Output.BiNormal = normalize(WorldBiNormal);

    return Output;


};