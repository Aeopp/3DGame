matrix World;
matrix View;
matrix Projection;

float DetailScale;

float Contract;
float Power;
float SpecularIntencity;
float CavityCoefficient;
float DetailDiffuseIntensity;
float DetailNormalIntensity;


texture DiffuseMap;
texture NormalMap;
texture CavityMap;
texture EmissiveMap;
texture DetailDiffuseMap;
texture DetailNormalMap;


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
    float3 WorldLocation : TEXCOORD4;
    float Depth : TEXCOORD5;
};


// 정점 쉐이더
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix WorldView, WorldViewProjection;

    WorldView = mul(World, View);
    WorldViewProjection = mul(WorldView, Projection);

    Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
    Out.UV = In.UV;
    Out.Normal = mul(float4(In.Normal.xyz, 0.f), World);
    Out.Tangent = mul(float4(In.Tangent.xyz, 0.f), World);
    Out.BiNormal = mul(float4(In.BiNormal.xyz, 0.f), World);
    
    Out.WorldLocation = mul(vector(In.Position.xyz, 1.f), World).xyz;
    Out.Depth = Out.Position.z / Out.Position.w;
    
    return Out;
}


struct PS_IN
{
    float3 Normal : TEXCOORD0;
    float3 Tangent : TEXCOORD1;
    float3 BiNormal : TEXCOORD2;
    float2 UV : TEXCOORD3;
    float3 WorldLocation : TEXCOORD4;
    float Depth : TEXCOORD5;
};


struct PS_OUT
{
    vector Albedo3_Contract1 : COLOR0;
    vector Normal3_Power1: COLOR1;
    vector WorldLocation3_Depth1 : COLOR2;
    vector Cavity1_CavityAlpha1_NULL1_NULL1 : COLOR3;
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
    WorldNormal = (WorldNormal * 0.5f)  + 0.5f;
    
    Out.Normal3_Power1 = float4(WorldNormal.xyz, Power);
    
    ///
    
    // 월드 위치와 NDC 깊이 패킹
    Out.WorldLocation3_Depth1 = float4(In.WorldLocation.xyz, In.Depth);
    ///
  
    CavityColor.rgb *= SpecularIntencity;
    float NULL1 = 1.f;
    float NULL2 = 1.f;
    
    Out.Cavity1_CavityAlpha1_NULL1_NULL1 = float4(CavityColor.r, CavityColor.a, NULL1, NULL2);
    
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