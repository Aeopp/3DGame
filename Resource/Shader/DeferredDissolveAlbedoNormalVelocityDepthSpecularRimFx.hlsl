matrix World;
matrix View;
matrix Projection;

matrix PrevWorldViewProjection;

float  DetailScale;
float3 AddAlbedo = float3(0, 0, 0);

float  Contract;
float  Power;
float  SpecularIntencity;
float  CavityCoefficient;
float  DetailDiffuseIntensity;
float  DetailNormalIntensity;
float  RimInnerWidth;
float  RimOuterWidth;
float OutlineRedFactor;

// Dissolve Burn Effect
float3 BurnColor = float3(1, 1, 1); 
float BurnSize = 0.15f;
float EmissionAmount = 2.f;


float SliceAmount;
texture DissolveMap;
texture BurnMap;
//

vector RimAmtColor;

texture DiffuseMap;
texture NormalMap;
texture CavityMap;
texture EmissiveMap;
texture DetailDiffuseMap;
texture DetailNormalMap;

sampler DissolveSampler = sampler_state
{
    texture = DissolveMap;
    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
}; 

sampler BurnSampler  = sampler_state
{
    texture = BurnMap;
    minfilter = linear;
    magfilter = linear;
    mipfilter = linear;
};

sampler DiffuseSampler = sampler_state
{
    texture = DiffuseMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
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
};
struct VS_OUT
{
    vector Position : POSITION;
    float3 Normal : TEXCOORD0;
    float3 Tangent : TEXCOORD1;
    float3 BiNormal : TEXCOORD2;
    float2 UV : TEXCOORD3;
    float4 ClipPosition : TEXCOORD4;
    float2 Velocity : TEXCOORD5;
};


// 정점 쉐이더
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix WorldView, WorldViewProjection;

    WorldView = mul(World, View);
    WorldViewProjection = mul(WorldView, Projection);

    Out.ClipPosition = Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
    Out.UV = In.UV;
    Out.Normal = mul(float4(In.Normal.xyz, 0.f), World);
    Out.Tangent = mul(float4(In.Tangent.xyz, 0.f), World);
    Out.BiNormal = mul(float4(In.BiNormal.xyz, 0.f), World);
    
    float4 PrevPosition = mul(float4(In.Position.xyz, 1.f), PrevWorldViewProjection);
    float3 Direction = Out.ClipPosition.xyz - PrevPosition.xyz;
    
    float2 Velocity = (Out.ClipPosition.xy / Out.ClipPosition.w) - (PrevPosition.xy / PrevPosition.w);
    Out.Velocity.xy = Velocity * 0.5f;
    Out.Velocity.y *= -1.f;
    
    return Out;
}


struct PS_IN
{
    float3 Normal : TEXCOORD0;
    float3 Tangent : TEXCOORD1;
    float3 BiNormal : TEXCOORD2;
    float2 UV : TEXCOORD3;
    float4 ClipPosition : TEXCOORD4;
    float2 Velocity : TEXCOORD5;
};


struct PS_OUT
{
    vector Albedo3_Contract1 : COLOR0;
    vector Normal3_Power1: COLOR1;
    vector Velocity2_None1_Depth1 : COLOR2;
    vector CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1Sampler : COLOR3;
};

PS_OUT AlbedoNormalWorldPosDepthSpecular(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 CavityColor = tex2D(CavitySampler, In.UV);
    CavityColor.rgb *= (CavityCoefficient);
    CavityColor.rgba = clamp(CavityColor.rgba, 0.0f, 1.0f);
    
    // 알베도 패킹....
    float4 DiffuseColor = tex2D(DiffuseSampler, In.UV);
    float4 DetailDiffuseColor = tex2D(DetailDiffuseSampler, (In.UV * DetailScale));
    DetailDiffuseColor *= DetailDiffuseIntensity;
    DiffuseColor = DiffuseColor * DetailDiffuseColor;
    DiffuseColor.rgb *= CavityColor.rgb;
    DiffuseColor.rgb += AddAlbedo.rgb;
    
    float Dissolve = tex2D(DissolveSampler, In.UV).r - SliceAmount;
    clip(Dissolve);
    float3 CurBurnColor = float3(1, 1, 1);
    
    if (Dissolve < BurnSize && SliceAmount > 0.0f)
    {
        CurBurnColor = tex2D(BurnSampler, float2(Dissolve * (1.f / BurnSize), 0.f)).rgb;
        CurBurnColor = CurBurnColor * BurnColor * EmissionAmount;
     
        // 여기서 선택지 컬러를 번 컬러로 할것 인가 컬러에 번 컬러를 곱할 것인가 ?? 
        // DiffuseColor.rgb  = CurBurnColor;
        // DiffuseColor.rgb *= CurBurnColor;
           DiffuseColor.rgb += CurBurnColor; 
    }
    
    Out.Albedo3_Contract1.rgb = DiffuseColor.rgb;
    Out.Albedo3_Contract1.a = Contract;
    // 
    
    // 노말과 스페큘러 하이라이팅 패킹...
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
    
    float3 WorldNormal = normalize(mul(TangentNormal, TBN));
    Out.Normal3_Power1 = float4(WorldNormal.xyz, Power);
    
    // 월드 위치와 NDC 깊이 패킹
    Out.Velocity2_None1_Depth1 = float4(In.Velocity.xy, OutlineRedFactor, In.ClipPosition.z / In.ClipPosition.w);
  
    CavityColor.rgb *= SpecularIntencity;
    Out.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1Sampler = float4(CavityColor.r, RimAmtColor.x, RimInnerWidth, RimOuterWidth);
    
    return Out;
}




technique Default_Device
{
    pass AlbedoNormalWorldPosDepthSpecular
    {
        zwriteenable     = true;
        alphablendenable = false;
        fillmode = solid;
        zenable  = true;
        cullmode = ccw;
        StencilEnable = true;
        StencilFunc = always;
        StencilPass = replace;
        StencilRef = 1;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 AlbedoNormalWorldPosDepthSpecular();
    }
}
