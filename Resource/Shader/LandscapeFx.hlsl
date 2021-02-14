matrix  World;
matrix  View;
matrix  Projection;

float4 LightColor;
float4 CameraLocation;
float4 LightDirection;
float4 RimAmtColor;
float  RimWidth;
// 1이 기본값이며 (외곽 림라이트) 0으로 갈수록 메쉬의 중심부로 림라이트가 이동.
float RimOuter;
float  Power;
float4 AmbientColor;

texture DiffuseMap;
texture TangentMap;
texture CavityMap;
texture EmissiveMap;


sampler DiffuseSampler  = sampler_state
{
    texture = DiffuseMap;

    minfilter = linear;
    magfilter = linear;
    mipfilter = linear;
};

sampler CavitySampler = sampler_state
{
    texture = CavityMap;

    minfilter = linear;
    magfilter = linear;
    mipfilter = linear;
};

sampler TangentSampler = sampler_state
{
    texture = TangentMap;

    minfilter = linear;
    magfilter = linear;
    mipfilter = linear;
};

sampler EmissiveSampler = sampler_state
{
    texture = EmissiveMap;

    minfilter = linear;
    magfilter = linear;
    mipfilter = linear;
};


struct VS_IN
{
	vector			Position : POSITION;
    float3          Normal : NORMAL;
    float3          Tangent : TANGENT;
    float3          BiNormal : BINORMAL;
	float2          UV : TEXCOORD0;
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
	VS_OUT Out = (VS_OUT)0;

	matrix		WorldView, WorldViewProjection;

	WorldView = mul(World, View);
	WorldViewProjection = mul(WorldView, Projection);

	Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
    Out.UV = In.UV;
    Out.Normal = mul( float4(In.Normal.xyz, 0.f), World);
    Out.Tangent = mul(float4(In.Tangent.xyz, 0.f), World);
    Out.BiNormal = mul(float4(In.BiNormal.xyz, 0.f), World);
    
    Out.WorldLocation = mul(vector(In.Position.xyz, 1.f), World).xyz;
    Out.ViewDirection = normalize(Out.WorldLocation.xyz - CameraLocation.xyz);
    
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
	vector			Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
    float3 Normal  = normalize(In.Normal);
    float3 Tangent = normalize(In.Tangent);
    float3 BiNormal = normalize(In.BiNormal);
    
    float3 TangentNormal = tex2D(TangentSampler, In.UV).xyz;
    TangentNormal = normalize(TangentNormal * 2 - 1);
    
    float3x3 TBN = float3x3(normalize(In.Tangent),
                            normalize(In.BiNormal),
                            normalize(In.Normal));
    
    TBN = transpose(TBN);
    float3 WorldNormal = normalize(mul(TBN, TangentNormal));
    
    In.ViewDirection = normalize(In.ViewDirection);
    
    float3 ToCamera = normalize(CameraLocation.xyz - In.WorldLocation.xyz);
    
    float RimAmt = abs(RimOuter - max(0.0, dot(Normal, ToCamera)));
    RimAmt = smoothstep(1.0f - saturate(RimWidth), 1.f, RimAmt);
   
   
    
    float3 LightDirectionNormal = normalize(LightDirection.xyz);
    
    float Specular = 0;
    
    float Diffuse = saturate(dot(-LightDirectionNormal, WorldNormal));
    
    float4 DiffuseColor = tex2D(DiffuseSampler, In.UV);
    float4 CavityColor = tex2D(CavitySampler, In.UV);
    
    float4 SpecularColor = CavityColor * 0.5;
    float4 CavityDiffuseColor = DiffuseColor * CavityColor;
    
    if (Diffuse.x > 0)
    {
        float3 HalfVec = normalize(LightDirectionNormal +In.ViewDirection);
        Specular = saturate(dot(HalfVec, WorldNormal));
        Specular = pow(abs(Specular),abs(Power));
    }
    
    float3 Ambient = AmbientColor.xyz;
    
    Out.Color = float4(LightColor.xyz * CavityDiffuseColor.xyz * Diffuse +
                    LightColor.xyz * SpecularColor.xyz * Specular, DiffuseColor.a);
    Out.Color.rgb += Ambient;
    Out.Color.rgba += RimAmt * RimAmtColor.rgba;
    
	return Out;
}

technique Default_Device
{
	pass	
	{
        alphablendenable = true;
        srcblend = srcalpha;
        destblend = invsrcalpha;

        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}