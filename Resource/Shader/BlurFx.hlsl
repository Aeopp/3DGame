texture DeferredTarget;
texture MotionBlur;

sampler DeferredTargetSampler = sampler_state
{
    texture = DeferredTarget;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};


sampler MotionBlurSampler = sampler_state
{
    texture = MotionBlur;

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

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    Out.Position = In.Position;
    Out.Position.w = 1.f;
    
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
    PS_OUT Out;
    
    float4 DeferredColor = tex2D(DeferredTargetSampler, In.UV);
    float4 MotionColor   = tex2D(MotionBlurSampler, In.UV);
    
    DeferredColor.rgb *= (1.f - MotionColor.a);
    Out.Color = float4(MotionColor.rgb + DeferredColor.rgb, 1.f);
    Out.Color = MotionColor.rgba;
    
    
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
        StencilEnable = false;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}