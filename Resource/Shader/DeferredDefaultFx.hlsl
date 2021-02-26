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

texture DiffuseMap;
texture NormalMap;
texture CavityMap;
texture EmissiveMap;
texture DetailDiffuseMap;
texture DetailNormalMap;

// UDN
//vec3 NormalBlend_UDN(vec3 n1, vec3 n2)
//{

//    return normalize(vec3(n1.xy + n2.xy, n1.z));
//}

// diffuse blend 
// basediffuse * detail diffuse 

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
    float3 ViewDirection : TEXCOORD4;
    float3 WorldLocation : TEXCOORD5;
};

// ���� ���̴�
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
    vector Albedo : COLOR1;
    vector Normal : COLOR2;
    vector WorldLocation : COLOR3;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
	
    float3 Normal = normalize(In.Normal);
    float3 Tangent = normalize(In.Tangent);
    float3 BiNormal = normalize(In.BiNormal);
    
    float3 TangentNormal = tex2D(NormalSampler, In.UV).xyz;
    TangentNormal = normalize((TangentNormal * 2.0) - 1.0);

    float3 DetailTangentNormal = tex2D(DetailNormalSampler, (In.UV * DetailScale)).xyz;
    DetailTangentNormal = normalize((DetailTangentNormal * 2.0) - 1.0);
    DetailTangentNormal *= DetailNormalIntensity;
    
    TangentNormal = normalize(float3(TangentNormal.xy + DetailTangentNormal.xy, TangentNormal.z));

    float3x3 TBN = float3x3(normalize(In.Tangent),
                            normalize(In.BiNormal),
                            normalize(In.Normal));
    
    TBN = transpose(TBN);
    float3 WorldNormal = normalize(mul(TBN, TangentNormal));
    
    In.ViewDirection = normalize(In.ViewDirection);
    
    float3 ToCamera = normalize(CameraLocation.xyz - In.WorldLocation.xyz);
    
    float Rim = max(0.0, dot(Normal, ToCamera));
    
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
    // ���� ����Ʈ.
    Diffuse = pow(((Diffuse * 0.5) + 0.5), Contract);
    // �� ���̵�
    Diffuse = ceil(Diffuse * 5.0) / 5.0f;
    
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
    Out.Albedo = Out.Color;
    Out.Normal = Out.Color;
    Out.WorldLocation = Out.Color;
    return Out;
}

technique Default_Device
{
    pass
    {
        alphablendenable = false;
        //srcblend = srcalpha;
        //destblend = invsrcalpha;

        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}