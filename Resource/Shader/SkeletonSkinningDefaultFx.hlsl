matrix World;
matrix View;
matrix Projection;

float DetailScale;
float4 LightColor;
float4 CameraLocation;
float4 LightDirection;
float4 RimAmtColor;
float RimOuterWidth;
float RimInnerWidth;
float Power;
float SpecularIntencity;
float CavityCoefficient;

float4 AmbientColor;
float Contract;
float DetailDiffuseIntensity;
float DetailNormalIntensity;
float AlphaAddtive; 

// VTF 텍스쳐
int VTFPitch;

texture DiffuseMap;
texture NormalMap;
texture CavityMap;
texture EmissiveMap;
texture DetailDiffuseMap;
texture DetailNormalMap;
texture VTF;
// UDN
//vec3 NormalBlend_UDN(vec3 n1, vec3 n2)
//{

//    return normalize(vec3(n1.xy + n2.xy, n1.z));
//}

// diffuse blend 
// basediffuse * detail diffuse 

sampler VTFSampler = sampler_state
{
    texture = VTF;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};

sampler DiffuseSampler = sampler_state
{
    texture = DiffuseMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
    addressu = wrap;
    addressv = wrap;
  

};

sampler CavitySampler = sampler_state
{
    texture = CavityMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;

};

sampler NormalSampler = sampler_state
{
    texture = NormalMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;

};

sampler EmissiveSampler = sampler_state
{
    texture = EmissiveMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;

};

sampler DetailDiffuseSampler = sampler_state
{
    texture = DetailDiffuseMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;

};

sampler DetailNormalSampler = sampler_state
{
    texture = DetailNormalMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;

};

struct VS_IN
{
    vector Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
    float2 UV : TEXCOORD0;
    float4 BoneIds : BLENDINDICES;
    float4 Weights : BLENDWEIGHT;
};

struct VS_OUT
{
    vector Position : POSITION;
    float3 Normal : TEXCOORD0;
    float3 Tangent : TEXCOORD1;
    float3 BiNormal : TEXCOORD2;
    float2 UV : TEXCOORD3;
    float3 ViewDirection : TEXCOORD4;
    float3 WorldLocation : TEXCOORD5;
};

// 정점 쉐이더
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    float4 AnimNormal = float4(0, 0, 0,0);
    float4 AnimTanget = float4(0, 0, 0, 0);
    float4 AnimBiNormal = float4(0, 0, 0, 0);
    float4 AnimPos = float4(0, 0, 0, 1);
    
    In.Position.w = 1.0f;
    
    float UVCorrection = 0.5f;
    float FVTFPitch = float(VTFPitch);
    int   IVTFPitch = int(VTFPitch);
    
    for (int i = 0; i < 4; ++i)
    {
        int Idx = In.BoneIds[i] *4; 
        
        float2 VTFUVRow0 = float2((float( Idx % IVTFPitch) +  UVCorrection) / FVTFPitch,
                                  (float( Idx / IVTFPitch) +  UVCorrection) / FVTFPitch);
        
        float2 VTFUVRow1 = float2((float((Idx +1) % IVTFPitch) + UVCorrection) / FVTFPitch,
                                  (float((Idx +1) / IVTFPitch) + UVCorrection) / FVTFPitch);
        
        float2 VTFUVRow2 = float2((float((Idx +2) % IVTFPitch) + UVCorrection) / FVTFPitch,
                                  (float((Idx +2) / IVTFPitch) + UVCorrection) / FVTFPitch);
        
        float2 VTFUVRow3 = float2((float( (Idx+3) % IVTFPitch) + UVCorrection) / FVTFPitch,
                                  (float( (Idx+3) / IVTFPitch) + UVCorrection) / FVTFPitch);
        
        float4x4 AnimMatrix =
        {
            tex2Dlod(VTFSampler, float4(VTFUVRow0, 0.f, 0.f)),
            tex2Dlod(VTFSampler, float4(VTFUVRow1, 0.f, 0.f)),
            tex2Dlod(VTFSampler, float4(VTFUVRow2, 0.f, 0.f)),
            tex2Dlod(VTFSampler, float4(VTFUVRow3, 0.f, 0.f))
        };
        
        AnimTanget += (mul(float4(In.Tangent, 0.f), AnimMatrix) * In.Weights[i]);
        AnimNormal += (mul(float4(In.Normal, 0.f), AnimMatrix) * In.Weights[i]);
        AnimBiNormal += (mul(float4(In.BiNormal, 0.f), AnimMatrix) * In.Weights[i]);
        AnimPos += (mul(In.Position, AnimMatrix) * In.Weights[i]);
    }
    
    
    matrix WorldView, WorldViewProjection;

    WorldView = mul(World, View);
    WorldViewProjection = mul(WorldView, Projection);

    Out.Position = mul(vector(AnimPos.xyz, 1.f), WorldViewProjection);
    Out.UV = In.UV;
    Out.Normal = mul(float4(AnimNormal.xyz, 0.f), World);
    Out.Tangent = mul(float4(AnimTanget.xyz, 0.f), World);
    Out.BiNormal = mul(float4(AnimBiNormal.xyz, 0.f), World);
    
    Out.WorldLocation = mul(float4(AnimPos.xyz, 1.f), World).xyz;
    Out.ViewDirection = normalize(CameraLocation.xyz - Out.WorldLocation.xyz);
    
    return Out;
}


struct PS_IN
{
    float3 Normal : TEXCOORD0;
    float3 Tangent : TEXCOORD1;
    float3 BiNormal : TEXCOORD2;
    float2 UV : TEXCOORD3;
    float3 ViewDirection : TEXCOORD4;
    float3 WorldLocation : TEXCOORD5;
};

struct PS_OUT
{
    vector Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
	
    
    float3 TangentNormal = tex2D(NormalSampler, In.UV).xyz;
    TangentNormal = normalize((TangentNormal * 2.0) - 1.0);
    TangentNormal.y *= -1.f;

    float3 DetailTangentNormal = tex2D(DetailNormalSampler, (In.UV * DetailScale)).xyz;
    DetailTangentNormal = normalize((DetailTangentNormal * 2.0) - 1.0);
    DetailTangentNormal.y *= -1.f;
    DetailTangentNormal *= DetailNormalIntensity;
    
    TangentNormal = normalize(float3(TangentNormal.xy + DetailTangentNormal.xy, TangentNormal.z));

    float3x3 TBN = float3x3(normalize(In.Tangent),
                            normalize(In.BiNormal),
                            normalize(In.Normal));
    
    float3 WorldNormal = normalize(mul(TangentNormal , TBN));
    
    In.ViewDirection = normalize(In.ViewDirection);
    
    float3 ToCamera = normalize(CameraLocation.xyz - In.WorldLocation.xyz);
    
    float Rim = max(0.0, dot(normalize(In.Normal), ToCamera));
    
    float RimAmtOuter = abs(1.f - Rim);
    float RimAmtInner = abs(0.f - Rim);
    
    float RimAmt = 0;
    
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
    
    float Specular = 0;
    
    float Diffuse = saturate(dot(-LightDirectionNormal, WorldNormal));
    // 하프 람버트.
    Diffuse = pow(((Diffuse * 0.5) + 0.5), Contract);
    // 셀 쉐이딩
    Diffuse = ceil(Diffuse * 10.0) / 10.0f;
    
    float4 DiffuseColor = tex2D(DiffuseSampler, In.UV);
    float4 DetailDiffuseColor = tex2D(DetailDiffuseSampler, (In.UV * DetailScale));
    DetailDiffuseColor *= DetailDiffuseIntensity;
    
    DiffuseColor = DiffuseColor * DetailDiffuseColor;
    float4 CavityColor = tex2D(CavitySampler, In.UV);
    CavityColor.rgb *= CavityCoefficient;
    CavityColor.rgba = clamp(CavityColor.rgba, 0.0f, 1.0f);
    float3 SpecularColor = CavityColor.rgb * SpecularIntencity;
    DiffuseColor.rgb *= CavityColor.rgb;
    
    float3 HalfVec = normalize((-LightDirectionNormal) + (In.ViewDirection));
    Specular = saturate(dot(HalfVec, WorldNormal));
    Specular = pow(abs(Specular), abs(Power));
    
    float3 Ambient = AmbientColor.xyz;
    
    Out.Color = float4(LightColor.xyz * DiffuseColor.rgb * Diffuse +
                    LightColor.xyz * SpecularColor.rgb * Specular, DiffuseColor.a);
    
    Out.Color.rgb += Ambient;
    Out.Color.rgba += RimAmt * RimAmtColor.rgba;
    Out.Color.a += AlphaAddtive;
    Out.Color.a = saturate(Out.Color.a);

    return Out;
}

technique Default_Device
{
    pass
    {
        alphablendenable = true;
        srcblend = srcalpha;
        destblend = invsrcalpha;
        zenable = true;
        zwriteenable = true;
        cullmode = ccw;
        fillmode = solid;

        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}