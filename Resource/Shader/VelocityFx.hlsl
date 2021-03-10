matrix WorldView;
matrix PrevWorldView;
matrix Projection;

struct VS_IN
{
    vector Position : POSITION;
    float3 Normal : NORMAL;
};

struct VS_OUT
{
    vector Position : POSITION;
    vector Velocity : TEXCOORD0;
};

// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    float4 CurPosition = 
      mul(float4(In.Position.xyz, 1.f), WorldView);
    
    float4 PrevPosition = 
        mul(float4(In.Position.xyz, 1.f),PrevWorldView);
    
    float4 Direction = CurPosition - PrevPosition;
    float3 Normal = mul(float4(In.Normal, 0.0f),WorldView);
    
    bool bFront = (0 <= dot(Normal, Direction.xyz));
    float4 SelectPosition = bFront ? CurPosition : PrevPosition;
    
    Out.Position = mul(SelectPosition, Projection);
    
    CurPosition = mul(CurPosition, Projection);
    PrevPosition = mul(PrevPosition, Projection);
    
    float2 Velocity = (CurPosition.xy / CurPosition.w)
                    - (PrevPosition.xy / PrevPosition.w);
    
    Out.Velocity.xy = Velocity * 0.5f;
    Out.Velocity.y *= -1.f;
    Out.Velocity.z = Out.Position.z;
    Out.Velocity.w = Out.Position.w;
    
    return Out;
}


struct PS_IN
{
    vector Velocity : TEXCOORD0;
};

struct PS_OUT
{
    vector Velocity : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.Velocity.xy = In.Velocity.xy;
    Out.Velocity.z = 1.0f;
    Out.Velocity.w = In.Velocity.z / In.Velocity.w;
    
    return Out;
}

technique Default_Device
{
    pass
    {
        alphablendenable = false;
        zenable = true;
        zwriteenable = false;
        cullmode = ccw;
        fillmode = solid;
        StencilEnable = false;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}