matrix World;
matrix View;
matrix Projection;

texture DiffuseMap;
texture WorldLocationDepth;

sampler DiffuseSampler = sampler_state
{
    texture = DiffuseMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;

    MaxAnisotropy = 16;
};

sampler WorldLocationDepthSampler = sampler_state
{
    texture = WorldLocationDepth;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;

    MaxAnisotropy = 16;
};


struct VS_IN
{
    vector Position : POSITION;
    float2 UV : TEXCOORD0;
};

struct VS_OUT
{
    vector Position : POSITION;
    float2 UV : TEXCOORD0;
    float2 ClipPosition : TEXCOORD1;
};

// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix WV = mul(World, View);
    matrix WVP = mul(WV, Projection);
    In.Position.w = 1.f;
    
    Out.Position = mul(In.Position, WVP);
    Out.ClipPosition = Out.Position.xy;
    Out.ClipPosition.xy /= Out.Position.w;
    
    Out.UV = In.UV;
    
    return Out;
};

struct PS_IN
{
    float2 UV : TEXCOORD0;
    float2 ClipPosition : TEXCOORD1;
};

struct PS_OUT
{
    vector Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    Out.Color = tex2D(DiffuseSampler, In.UV) ;
    In.ClipPosition.xy = In.ClipPosition.xy * 0.5f + 0.5f;
    In.ClipPosition.y *= -1.f;
    
    if (tex2D(WorldLocationDepthSampler, In.ClipPosition.xy).w !=1.f)
    {
        discard;
    }
    
    return Out;
}

technique Default_Device
{
    pass
    {
        alphablendenable = false;
        zwriteenable     = false;
        cullmode = ccw;
        zenable = false;
        zwriteenable = false;
        fillmode = solid;

        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}
