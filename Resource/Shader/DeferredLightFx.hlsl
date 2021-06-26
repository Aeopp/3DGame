float4x4 View;
float4x4 Projection;
float4x4 InverseViewProjection;
float4x4 ViewProjection;

float4x4 LightViewProjection;
float4   LightLocation;

float4x4 PrevInverseViewProjection;
float4x4 PrevView; 
float4x4 PrevProjection;
float4x4 PrevViewProjection;

float4 CameraLocation;
float4 LightDirection;

float4 LightColor; 
float3 AmbientColor = float3(0.01, 0.01, 0.01);
float3 FogColor;
float FogDistance;

float ShadowDepthMapHeight;
float ShadowDepthMapWidth;
float ShadowDepthBias;

bool bPCF = true;

// 외곽선 검출
float OutlineMask[9] = 
    {   -1,-1,-1 ,
        -1, 8,-1 ,
        -1,-1,-1    };
float OuelineCoord[3] = { -1, 0, +1 };
float MapSizeX;
float MapSizeY;
// 
#define PCFCount 3

texture Albedo3_Contract1;
texture Normal3_Power1;
texture Velocity2_OutlineRedFactor_Depth1;
texture CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1;
texture ShadowDepth;
texture EmissiveMap;


sampler EmissiveSampler = sampler_state
{
    texture = EmissiveMap;
    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};


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

sampler Velocity2_OutlineRedFactor_Depth1Sampler = sampler_state
{
    texture = Velocity2_OutlineRedFactor_Depth1;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
};

sampler CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1Sampler = sampler_state
{
    texture = CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1;

   
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

    addressu = border;
    addressv = border;
    bordercolor = 0xffffffff;
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
    vector BackBufferColor : COLOR0;
    vector DeferredTargetColor : COLOR1;
};


PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
        
    float4 Albedo3_Contract1 = tex2D(Albedo3_Contract1Sampler, In.UV);
    float4 Velocity_OutlineRedFactor_Depth = tex2D(Velocity2_OutlineRedFactor_Depth1Sampler, In.UV);
    float Depth = Velocity_OutlineRedFactor_Depth.a;
    float OutlineRedAddtive = Velocity_OutlineRedFactor_Depth.z;
    
    float4 CurrentNDC = float4(In.UV.x * 2.f - 1.f, (1.f-In.UV.y) * 2.f - 1.f, Depth, 1.f);
    // view-projection 역으로 변환합니다. 
    float4 WorldLocation = mul(CurrentNDC, InverseViewProjection);
    WorldLocation /= WorldLocation.w;
    
    
    float3 Albedo   = Albedo3_Contract1.rgb;
    
    float4 LightClipPosition = mul(float4(WorldLocation.xyz, 1.f), LightViewProjection);
    LightClipPosition.xyz = LightClipPosition.xyz / LightClipPosition.w;
    LightClipPosition.y *= -1.f;
    LightClipPosition.xy *= 0.5f;
    LightClipPosition.xy += 0.5f;
    
    float ShadowFactor = 1.15f;
    
    if (bPCF)
    {
        if (saturate(LightClipPosition.z) == LightClipPosition.z)
        {
            float LookUpCount = (PCFCount * 2.0f + 1) * (PCFCount * 2.0f + 1);

            float Shadow = 0.0;
            float TexelSizeU = 1.0 / ShadowDepthMapWidth;
            float TexelSizeV = 1.0 / ShadowDepthMapHeight;
            for (int x = -PCFCount; x <= PCFCount; ++x)
            {
                for (int y = -PCFCount; y <= PCFCount; ++y)
                {
                    float2 UVOffset = float2(x * TexelSizeU, y * TexelSizeV);

                    float pcfDepth = tex2D(ShadowDepthSampler, LightClipPosition.xy + UVOffset).x;
                    if (LightClipPosition.z > (pcfDepth + ShadowDepthBias))
                    {
                        Shadow += 1.0f;
                    }
                }
            }
            Shadow /= LookUpCount;
            ShadowFactor -= Shadow;
        }
    }
    else
    {
        if (saturate(LightClipPosition.z) == LightClipPosition.z)
        {
           const float Depth = tex2D(ShadowDepthSampler, LightClipPosition.xy).x;
           if (LightClipPosition.z > (Depth + ShadowDepthBias))
           {
               ShadowFactor -= 1.0f;
           };
        }
    };
    
   
    float4 Normal3_Power1 = tex2D(Normal3_Power1Sampler, In.UV);
    float4 _CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1 = tex2D(CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1Sampler, In.UV);
        
    float  Contract = Albedo3_Contract1.a;
        
    float3 Normal = Normal3_Power1.rgb;
    // 외곽선 추출 시작.
    // 월드 노말에서 추출 . 
    float3 OutlineColor = 0;
    float3 Ret = 0;
    for (int i = 0; i < 9; ++i)
    {
        float2 CurrentCoordAddtive = float2(
        OuelineCoord[i % 3] / MapSizeX, OuelineCoord[i / 3] / MapSizeY);
        
        float3 ViewNormal = mul(float4(tex2D(Normal3_Power1Sampler, In.UV + CurrentCoordAddtive).xyz, 0.f), View).xyz;
        
        OutlineColor += (OutlineMask[i] * ViewNormal).xyz;
    }
    float Gray = 1 - (OutlineColor.r * 0.3 + OutlineColor.g * 0.59 + OutlineColor.b * 0.11);
    Ret = float3(Gray + ( (1.f - Gray) * OutlineRedAddtive) , Gray, Gray);
    
    float Power = Normal3_Power1.a;
       
    
    float3 CavityColor = _CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.rrr;
    
    float3 RimLightColor = _CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.ggg;
    float RimInnerWidth = _CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.b;
    float RimOuterWidth = _CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.a;
    
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
    // Diffuse = ceil(Diffuse * 6.f) / 6.f;
    float3 HalfVec = normalize((-LightDirectionNormal) + ViewDirection);
    Specular = saturate(dot(HalfVec, Normal));
    Specular = pow((Specular),(Power));
    
    Out.BackBufferColor = float4(Albedo.rgb * LightColor.rgb * Diffuse +
                                CavityColor.rgb * LightColor.rgb * Specular, 1.0f);
    
    Out.BackBufferColor.rgb += RimAmt * RimLightColor.rgb;
    Out.BackBufferColor.rgb += AmbientColor.rgb;
    ShadowFactor = saturate(ShadowFactor);
    Out.BackBufferColor.rgb *= ShadowFactor;
    float Distance = length(WorldLocation.xyz - CameraLocation.xyz);
    // 가까우면 1 멀면 0 
    float FogFactor = saturate((FogDistance - Distance) / FogDistance);
    Out.BackBufferColor.rgb = Out.BackBufferColor.rgb * (FogFactor) + ((1.0f - FogFactor) * FogColor);
    // 외곽선 더하기
    Out.BackBufferColor.rgb *=Ret;
     Out.BackBufferColor.rgb += tex2D(EmissiveSampler, In.UV).rgb;
    Out.DeferredTargetColor = Out.BackBufferColor;
    
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