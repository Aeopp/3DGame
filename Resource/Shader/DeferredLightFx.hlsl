float4x4 View;
float4x4 Projection;

float4 CameraLocation;
float4 LightDirection;

float4 LightColor; 
float3 AmbientColor = float3(0.1, 0.1, 0.1);

texture Albedo3_Contract1;
texture Normal3_Power1;
texture WorldPos3_Depth1;
texture CavityRGB1_CavityAlpha1;
texture RimRGB1_InnerWidth1_OuterWidth1;

sampler Albedo3_Contract1Sampler= sampler_state
{
    texture = Albedo3_Contract1;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
    MaxAnisotropy = 16;
};

sampler Normal3_Power1Sampler = sampler_state
{
    texture = Normal3_Power1;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
    
};

sampler WorldPos3_Depth1Sampler = sampler_state
{
    texture = WorldPos3_Depth1;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};

sampler CavityRGB1_CavityAlpha1Sampler = sampler_state
{
    texture = CavityRGB1_CavityAlpha1;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};

sampler RimRGB1_InnerWidth1_OuterWidth1Sampler = sampler_state
{
    texture = RimRGB1_InnerWidth1_OuterWidth1;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
    
};

//struct VS_IN
//{
//    vector Position : POSITION;
//    float2 UV : TEXCOORD0;
//};
//struct VS_OUT
//{
//    vector Position : POSITION;
//    float2 UV : TEXCOORD0;
//};

//VS_OUT VS_MAIN(VS_IN In)
//{
//    VS_OUT Out;
//    Out.UV = In.UV;
//    Out.Position = float4(In.Position.xyz, 1.f);
    
//    return Out;
//}

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
   
    
    float4 Albedo3_Contract1 = tex2D(Albedo3_Contract1Sampler, In.UV);
    float4 Normal3_Power1 = tex2D(Normal3_Power1Sampler, In.UV);  
    float4 WorldPos3_Depth1=tex2D(WorldPos3_Depth1Sampler, In.UV);
    float4 CavityRGB1_CavityAlpha1 = tex2D(CavityRGB1_CavityAlpha1Sampler, In.UV);
    float4 RimRGB1_InnerWidth1_OuterWidth1  = tex2D(RimRGB1_InnerWidth1_OuterWidth1Sampler, In.UV);
    
    float3 Albedo   = Albedo3_Contract1.rgb;
    float  Contract = Albedo3_Contract1.a;
    
    float3 Normal = Normal3_Power1.rgb;
    Normal *= 2.0f;
    Normal -= 1.f;
    float Power = Normal3_Power1.a;
    
    float3 WorldLocation = WorldPos3_Depth1.rgb;
    WorldLocation *= 2.0f;
    WorldLocation -= 1.f;
    float Depth = WorldPos3_Depth1.a;
    
    float3 CavityColor = CavityRGB1_CavityAlpha1.rrr;
    float  CavityAlpha = CavityRGB1_CavityAlpha1.g;
    
    float3 RimLightColor = RimRGB1_InnerWidth1_OuterWidth1.aaa;
    float RimInnerWidth  = RimRGB1_InnerWidth1_OuterWidth1.g;
    float RimOuterWidth  = RimRGB1_InnerWidth1_OuterWidth1.b;
    
    // float3 ViewDirection = CameraLocation.xyz - WorldLocation.xyz;
    // float Rim = max(0.0, dot(normalize(Normal), ViewDirection)));
    
    float3 LightDirectionNormal = normalize(LightDirection.xyz);
    float Specular = 0.f;
    float Diffuse = saturate(dot(-LightDirectionNormal.xyz, Normal));
    Diffuse = pow(((Diffuse * 0.5) + 0.5), Contract);
    Diffuse = ceil(Diffuse * 5.0) / 5.0f;
    
    Out.Color = float4(Albedo *LightColor.rgb* Diffuse +AmbientColor,1.f);
    
    // Out.Color = float4(1, 0, 0, 1);
    
    
    //// ?? 
    //Out.Color = float4(Albedo, 1.f);
    
    //Out.Color = float4(1, 1, 1, 1);
    
    
    return Out;
}




technique DeferredLight
{
    pass DeferredLightPass
    {
        zwriteenable = false;
        alphablendenable = false;
        fillmode = solid;
        zenable = false;
        cullmode = ccw;

        // vertexshader = compile vs_3_0 VS_MAIN();
        Vertexshader = NULL; 
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}