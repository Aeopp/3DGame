float4x4 View;
float4x4 Projection;
float4x4 LightViewProjection;
float4   LightLocation;

float4 CameraLocation;
float4 LightDirection;

float4 LightColor; 
float3 AmbientColor = float3(0.01, 0.01, 0.01);

texture Albedo3_Contract1;
texture Normal3_Power1;
texture WorldPos3_Depth1;
texture CavityRGB1_CavityAlpha1;
texture RimRGB1_InnerWidth1_OuterWidth1;
texture ShadowDepth;

sampler Albedo3_Contract1Sampler= sampler_state
{
    texture = Albedo3_Contract1;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
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

sampler ShadowDepthSampler = sampler_state
{
    texture = ShadowDepth;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};

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
    float  Depth = WorldPos3_Depth1.a;
    
    float3 CavityColor = CavityRGB1_CavityAlpha1.rrr;
    float  CavityAlpha = CavityRGB1_CavityAlpha1.g;
    
    float3 RimLightColor  = RimRGB1_InnerWidth1_OuterWidth1.rrr;
    float  RimInnerWidth  = RimRGB1_InnerWidth1_OuterWidth1.g;
    float  RimOuterWidth  = RimRGB1_InnerWidth1_OuterWidth1.b;
    
    float3 ViewDirection = normalize(CameraLocation.xyz - WorldLocation.xyz);
    
    float Rim = max(0.0f, dot(normalize(Normal), ViewDirection));
    
    float RimAmtOuter = abs(1.f - Rim);
    float RimAmtInner = abs(0.f-  Rim);
    
    float RimAmt = 0.f;
    
    if (RimAmtOuter > RimAmtInner)
    {
        RimAmt = RimAmtOuter;
        RimAmt = smoothstep(1.0f - saturate(RimOuterWidth), 1.f, RimAmt);
    }
    else
    {
        RimAmt = RimAmtInner;
        RimAmt = smoothstep(1.0f - saturate(RimInnerWidth), 1.f, RimAmt);
    }
    
    float3 LightDirectionNormal = normalize(LightDirection.xyz);
    
    float Specular = 0.0f;
    
    float Diffuse = saturate(dot(-LightDirectionNormal, Normal));
    Diffuse = pow(((Diffuse * 0.5) + 0.5), Contract);
    Diffuse = ceil(Diffuse * 5.0) / 5.0f;
    
    float3 HalfVec = normalize((-LightDirectionNormal) + ViewDirection);
    Specular = saturate(dot(HalfVec, Normal));
    Specular = pow(abs(Specular), abs(Power));
    
    Out.Color = float4(   Albedo.rgb * LightColor.rgb * Diffuse +
                         CavityColor.rgb * LightColor.rgb * Specular, 1.0f);
    
    Out.Color.rgb += AmbientColor.rgb;
    Out.Color.rgb += RimAmt * RimLightColor.rgb;
    
    //matrix LightViewProjection = mul(LightView, LightProjection);
    //float4 LightClipPosition = mul(float4(WorldLocation, 1.f), LightViewProjection);
    //LightClipPosition.xy / LightClipPosition.w;
    //float aa = tex2D(ShadowDepthSampler, LightClipPosition.xy);
    
    
    return Out;
}




technique DeferredLight
{
    pass DeferredLightPass
    {
        zwriteenable = false;
        alphablendenable = false;
        srcblend = srcalpha;
        destblend = invsrcalpha;
        fillmode = solid;
        zenable = false;
        cullmode = ccw;

        Vertexshader = NULL; 
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}