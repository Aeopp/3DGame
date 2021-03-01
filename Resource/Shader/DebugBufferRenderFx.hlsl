
texture BaseTexture;
matrix Ortho;
matrix ScreenMatrix;
sampler BaseSampler = sampler_state
{
    texture = BaseTexture;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
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
    Out.Position = mul(float4(In.Position.xyz, 1.f), ScreenMatrix);
    Out.Position = mul(float4(Out.Position.xyz, 1.f), Ortho);
    Out.UV = In.UV;
    
    return Out;
}

struct PS_IN
{
    float2 UV : TEXCOORD0;
};

struct PS_OUT
{
    vector COLOR : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    Out.COLOR = tex2D(BaseSampler, In.UV); 
    return Out;
}

technique Default_Device
{
    pass
    {
        alphablendenable = false;
        zenable = false;
        zwriteenable = false;
        cullmode = none;
        fillmode = solid;
        stencilenable = false; 
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}