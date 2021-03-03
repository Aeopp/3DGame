float4x4 View;
float4x4 Projection;
float4x4 LightViewProjection;
float4   LightLocation;

float4 CameraLocation;
float4 LightDirection;

float4 LightColor; 
float3 AmbientColor = float3(0.01, 0.01, 0.01);
float3 FogColor = float3(1.f, 1.f, 1.f);
float  FogDistance = 10000.f;

float ShadowDepthBias;

float ShadowDepthMapSize;
#define PCFCount 3

texture Albedo3_Contract1;
texture Normal3_Power1;
texture WorldPos3_Depth1;
texture CavityRGB1_CavityAlpha1;
texture RimRGB1_InnerWidth1_OuterWidth1;
texture ShadowDepth;

sampler Albedo3_Contract1Sampler= sampler_state
{
    texture = Albedo3_Contract1;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};

sampler Normal3_Power1Sampler = sampler_state
{
    texture = Normal3_Power1;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
    
};

sampler WorldPos3_Depth1Sampler = sampler_state
{
    texture = WorldPos3_Depth1;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};

sampler CavityRGB1_CavityAlpha1Sampler = sampler_state
{
    texture = CavityRGB1_CavityAlpha1;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};

sampler RimRGB1_InnerWidth1_OuterWidth1Sampler = sampler_state
{
    texture = RimRGB1_InnerWidth1_OuterWidth1;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};

sampler ShadowDepthSampler = sampler_state
{
    texture = ShadowDepth;

  
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

VS_OUT VS_MAIN(VS_IN In) 
{
    VS_OUT Out = (VS_OUT)0;
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
    PS_OUT Out = (PS_OUT) 0;
        
    float4 Albedo3_Contract1 = tex2D(Albedo3_Contract1Sampler, In.UV);
    float4 WorldPos3_Depth1 = tex2D(WorldPos3_Depth1Sampler, In.UV);
    float3 WorldLocation = WorldPos3_Depth1.rgb;
    float3 Albedo   = Albedo3_Contract1.rgb;
    
    float4 LightClipPosition = mul(float4(WorldLocation, 1.f), LightViewProjection);
    LightClipPosition.xy = LightClipPosition.xy / LightClipPosition.w;
    LightClipPosition.y *= -1.f;
    LightClipPosition.xy *= 0.5f;
    LightClipPosition.xy += 0.5f;
    
    float ShadowFactor = 1.15f;
    
    if (LightClipPosition.x >= 0.0f && LightClipPosition.x <= 1.0f
         && LightClipPosition.y >= 0.0f && LightClipPosition.y <= 1.0f)
    {
        float LookUpCount = (PCFCount * 2.0f + 1) * (PCFCount * 2.0f+ 1);
        
        float Shadow = 0.0;
        float2 TexelSize = 1.0 / ShadowDepthMapSize;
        for (int x = -PCFCount; x <= PCFCount; ++x)
        {
            for (int y = -PCFCount; y <= PCFCount; ++y)
            {
                float pcfDepth = tex2D(ShadowDepthSampler, LightClipPosition.xy + float2(x, y) * TexelSize).r;
                if (LightClipPosition.z > (pcfDepth + ShadowDepthBias))
                {
                    Shadow += 1.0f;
                }
            }
        }
        Shadow /= LookUpCount;
        ShadowFactor -= Shadow;
    }
    
    float4 Normal3_Power1 = tex2D(Normal3_Power1Sampler, In.UV);
    float4 CavityRGB1_CavityAlpha1 = tex2D(CavityRGB1_CavityAlpha1Sampler, In.UV);
    float4 RimRGB1_InnerWidth1_OuterWidth1 = tex2D(RimRGB1_InnerWidth1_OuterWidth1Sampler, In.UV);
        
    float Contract = Albedo3_Contract1.a;
        
    float3 Normal = Normal3_Power1.rgb;
    Normal *= 2.0f;
    Normal -= 1.f;
    float Power = Normal3_Power1.a;
    
       
    float Depth = WorldPos3_Depth1.a;
    
    float3 CavityColor = CavityRGB1_CavityAlpha1.rrr;
    float CavityAlpha = CavityRGB1_CavityAlpha1.g;
    
    float3 RimLightColor = RimRGB1_InnerWidth1_OuterWidth1.rrr;
    float RimInnerWidth = RimRGB1_InnerWidth1_OuterWidth1.g;
    float RimOuterWidth = RimRGB1_InnerWidth1_OuterWidth1.b;
    
    float3 ViewDirection = normalize(CameraLocation.xyz - WorldLocation.xyz);
    
    float Rim = max(0.0f, dot(normalize(Normal), ViewDirection));
    
    float RimAmtOuter = abs(1.f - Rim);
    float RimAmtInner = abs(0.f - Rim);
    
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
    
    Out.Color = float4(Albedo.rgb * LightColor.rgb * Diffuse +
                       CavityColor.rgb * LightColor.rgb * Specular, 1.0f);
    
    Out.Color.rgb += RimAmt * RimLightColor.rgb;
    ShadowFactor = saturate(ShadowFactor);
    Out.Color.rgb *= ShadowFactor;
    Out.Color.rgb += AmbientColor.rgb;
    
    float Distance =  length(WorldLocation.xyz - CameraLocation.xyz);
    // 가까우면 1 멀면 0 
    float FogFactor = saturate((FogDistance - Distance) / FogDistance);
    
    Out.Color.rgb += (FogFactor * FogColor);
    
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

        Vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}