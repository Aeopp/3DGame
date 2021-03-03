matrix World;
matrix LightViewProjection;

struct VS_IN
{
    vector Position : POSITION;
};

struct VS_OUT
{
    vector Position : POSITION;
    vector WorldPosition : TEXCOORD0;
};

// ���� ���̴�
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix WVP = mul(World, LightViewProjection);
    
    Out.Position = mul(vector(In.Position.xyz, 1.f), WVP);
    Out.WorldPosition = mul(float4(In.Position.xyz, 1.f), World);
    
    return Out;
}

struct PS_IN
{
    vector WorldPosition : TEXCOORD0;
};

struct PS_OUT
{
    vector ShadowDepth : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 Depth = mul(In.WorldPosition, LightViewProjection);
    Depth /= Depth.w;
    Out.ShadowDepth = float4(Depth.zzzz);
    
    return Out;
}




technique Default_Device
{
    pass
    {
        alphablendenable = false;
        zenable = true;
        zwriteenable = true;
        cullmode = ccw;
        fillmode = solid;
        stencilenable = false; 
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}