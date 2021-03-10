texture DeferredTarget;
texture VelocityMap;
texture Velocity2_None1_Depth1;

sampler Velocity2_None1_Depth1Sampler = sampler_state
{
    texture = Velocity2_None1_Depth1;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};

sampler DeferredTargetSampler = sampler_state
{
    texture = DeferredTarget;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};      

sampler VelocityMap_Sampler = sampler_state
{
    texture = VelocityMap;

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
    Out.UV = In.UV;
    
    return Out;
}

struct PS_IN
{
    float2 UV : TEXCOORD0; 
};

struct PS_OUT
{
    vector BackBufferColor : COLOR0;
    vector MotionColor : COLOR1;
    
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
  
    float4  Color = float4(0, 0, 0,0);
    /////////
    int NumBlurSample = 8;
    float4 Velocity = tex2D(VelocityMap_Sampler, In.UV);
   
    if(Velocity.x!=0.0f)
    {
        Velocity.xy /= (float) NumBlurSample;
        int iCnt = 1;
    
        float4 BColor;
        for (int i = iCnt; i < NumBlurSample; ++i)
        {
            BColor = tex2D(DeferredTargetSampler, In.UV + (Velocity.xy * (float) i));
            BColor.a = 1.f;
            float BColorDepth = tex2D(Velocity2_None1_Depth1Sampler, In.UV + (Velocity.xy * (float) i)).a;
        
            if (Velocity.a < (BColorDepth + 0.04f))
            {
                iCnt++;
                Color += BColor;
            }
        }
        Color /= (float)NumBlurSample;
    }
   
    Out.MotionColor = Color;
    Out.BackBufferColor= Color;
    /////// 
    return Out;
};

technique Default_Device
{
    pass
    {
        alphablendenable = true;
        srcblend  = one;
        destblend = invsrcalpha;
        zenable = false;
        zwriteenable = false;
        cullmode = ccw;
        fillmode = solid;
        stencilenable = false; 
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}