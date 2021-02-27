matrix World;
matrix View;
matrix Projection;

float4 RimAmtColor;
float RimOuterWidth;
float RimInnerWidth;

struct VS_IN
{
    vector Position :  POSITION;
};

struct VS_OUT
{
    vector Position : POSITION;
};


// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix WorldView, WorldViewProjection;

    WorldView = mul(World, View);
    WorldViewProjection = mul(WorldView, Projection);

    Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
    
    return Out;
}


struct PS_IN
{
    
};


struct PS_OUT
{
    vector RimColor1_InnerWidth1_OuterWidth1_NULL1: COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
   
    Out.RimColor1_InnerWidth1_OuterWidth1_NULL1 = 
    float4(RimAmtColor.r,RimInnerWidth,RimOuterWidth,1.f);
    
    
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
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}