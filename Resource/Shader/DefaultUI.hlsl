texture ColorMap;
matrix  UIMatrix;
float3 AddColor;
float CoolTimeHeight;
int Flag = 1;
float AlphaFactor;

sampler ColorSampler = sampler_state
{
    texture = ColorMap;

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
};

// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    Out.Position = In.Position;
    Out.Position.w = 1.f;
    Out.Position = mul(Out.Position, UIMatrix);
    
    Out.UV = In.UV;
    
    return Out;
}


struct PS_IN
{
    float2 UV : TEXCOORD0;
};

struct PS_OUT
{
    vector Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    Out.Color = tex2D(ColorSampler, In.UV);
    
    if ((1.0f-In.UV.y) < CoolTimeHeight)
    {
        if (Flag ==1)
        {
            Out.Color.rgb *= AddColor;
        }
    }
    else
    {
        Out.Color.rgb *= 0.4f;
    }
    
    Out.Color.a *= AlphaFactor;
    
    return Out;
}

technique Default_Device
{
    pass
    {
        alphablendenable = true;
        srcblend = srcalpha;
        destblend = invsrcalpha;
        zenable = false;
        zwriteenable = false;
        cullmode = ccw;
        fillmode = solid;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}