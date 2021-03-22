

// ¹ÙÀÎµù
texture Emissive;
float TexSizeX;
float TexSizeY;
//

float4 GaussFilterY[7] =
{
    0.0, -3.0, 0.0, 1.0 / 64.0,
   0.0, -2.0, 0.0, 6.0 / 64.0,
   0.0, -1.0, 0.0, 15.0 / 64.0,
   0.0, 0.0, 0.0, 20.0 / 64.0,
   0.0, 1.0, 0.0, 15.0 / 64.0,
   0.0, 2.0, 0.0, 6.0 / 64.0,
   0.0, 3.0, 0.0, 1.0 / 64.0
};


float4 GaussFilterX[7] =
{
    -3.0, 0.0, 0.0, 1.0 / 64.0,
   -2.0, 0.0, 0.0, 6.0 / 64.0,
   -1.0, 0.0, 0.0, 15.0 / 64.0,
   0.0, 0.0, 0.0, 20.0 / 64.0,
   1.0, 0.0, 0.0, 15.0 / 64.0,
   2.0, 0.0, 0.0, 6.0 / 64.0,
   3.0, 0.0, 0.0, 1.0 / 64.0
};

sampler EmissiveSampler = sampler_state
{
    texture = Emissive;

    minfilter = linear;
    magfilter = linear;
    mipfilter = linear;
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
    vector Blur : COLOR0;
};

PS_OUT PS_BlurX(PS_IN In)
{    
    PS_OUT Out;
    float4 Color = 0.0;
    float2 TexOffset = float2(1.f /TexSizeX,1.f/TexSizeY);
    
    int i;
    for (i = 0; i < 7; i++)
    {
        Color += (tex2D(EmissiveSampler, float2(In.UV.x + (GaussFilterX[i].x * TexOffset.x),
                                              In.UV.y + (GaussFilterX[i].y * TexOffset.y))) *
                                    GaussFilterX[i].w);

    }  
   
    
    Out.Blur = Color;
    
    return Out;
}

PS_OUT PS_BlurY(PS_IN In)
{
    PS_OUT Out;
    
    float4 Color = 0.0;
    float2 TexOffset = float2(1.f / TexSizeX, 1.f / TexSizeY);

    int i;
    for (i = 0; i < 7; i++)
    {
        Color += (tex2D(EmissiveSampler, float2(In.UV.x + (GaussFilterY[i].x * TexOffset.x),
                                     In.UV.y + (GaussFilterY[i].y * TexOffset.y))) *
                    GaussFilterY[i].w);
    }
   
    Out.Blur = Color;
    
    return Out;
};


technique Default_Device
{
    pass
    {
        //alphablendenable = true;
        //srcblend = srcalpha;
        //destblend = invsrcalpha;
        zenable = false;
        zwriteenable = false;
        cullmode = none;
        fillmode = solid;
        StencilEnable = false;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_BlurX();
    }

    pass
    {
        //alphablendenable = true;
        //srcblend = srcalpha;
        //destblend = invsrcalpha;
        zenable = false;
        zwriteenable = false;
        cullmode = none;
        fillmode = solid;
        StencilEnable = false;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_BlurY();
    }
}