matrix World;
matrix View;
matrix Projection;


float AlphaFactor = 1.f;
float Brightness = 1.f;
float Time;
float2 GradientUVOffsetFactor;

texture DiffuseMap;

texture PatternMap;
texture AddColorMap;
texture UVDistorMap;
texture GradientMap;


// ����Ʈ ��ƼŬ
texture VelocityNoneDepthMap;
float SoftParticleDepthScale = 30000.f;
float Far = 1000.f;
//
sampler VelocityNoneDepthSampler = sampler_state
{
    texture = VelocityNoneDepthMap;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};


sampler GradientSampler = sampler_state
{
    texture = GradientMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
    addressu = clamp;
    addressv = clamp;
};

sampler UVDistorSampler = sampler_state
{
    texture = UVDistorMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
    addressu = wrap;
    addressv = wrap;
};

sampler AddColorSampler = sampler_state
{
    texture = AddColorMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
    addressu = wrap;
    addressv = wrap;
};

sampler PatternSampler = sampler_state
{
    texture = PatternMap;

    minfilter = anisotropic;
    magfilter = anisotropic;
    mipfilter = anisotropic;
    MaxAnisotropy = 16;
    addressu = wrap;
    addressv = wrap;
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

struct VS_IN
{
    vector Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct VS_OUT
{
    vector Position : POSITION;
    float2 UV : TEXCOORD0;
    float4 ClipPosition : TEXCOORD1;
};

// ���� ���̴�
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
  
    In.Position.w = 1.0f;
  
    matrix WorldView, WorldViewProjection;

    WorldView = mul(World, View);
    WorldViewProjection = mul(WorldView, Projection);

    Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
    Out.ClipPosition = Out.Position;
    Out.UV = In.UV;
    // Out.Normal = mul(float4(In.Normal.xyz, 0.f), World);
    
    return Out;
}


struct PS_IN
{
    float2 UV : TEXCOORD0;
    float4 ClipPosition : TEXCOORD1;
};

struct PS_OUT
{
    vector Color : COLOR0;
};


PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float2 CurUV = (In.UV + Time);
    float4 Noise = tex2D(UVDistorSampler, CurUV);
    float2 GradientUV = float2(1.f -  (Time * GradientUVOffsetFactor.x) , 
    0.f + (Time * GradientUVOffsetFactor.y));
    float4 Gradient = tex2D(GradientSampler, GradientUV);
    
    float4 DiffuseColor = tex2D(DiffuseSampler, In.UV + Noise.xy);
    float4 AddColor = tex2D(AddColorSampler, In.UV);
    float4 Pattern = tex2D(PatternSampler, In.UV + Noise.xy);
    
    Out.Color = (DiffuseColor + AddColor) * Pattern;
    Out.Color *= Gradient;
    
    Out.Color.rgb *= Brightness;
    Out.Color.a *= AlphaFactor;
    
    
    float2 vDepthUV = (float2) 0.f;

	// -1 -> 0, 1 -> 1 (z������ ���Ŀ� ���� ��ǥ�� �ؽ��� uv�� ��ȯ�ϴ� �۾�)
    vDepthUV.x = (In.ClipPosition.x / In.ClipPosition.w) * 0.5f + 0.5f;

	// 1 -> 0, -1 -> 1 (z������ ���Ŀ� ���� ��ǥ�� �ؽ��� uv�� ��ȯ�ϴ� �۾�)
    vDepthUV.y = (In.ClipPosition.y / In.ClipPosition.w) * -0.5f + 0.5f;

    float fViewZ = tex2D(VelocityNoneDepthSampler, vDepthUV).w;
    
    Out.Color.a = Out.Color.a * saturate(((fViewZ - In.ClipPosition.z / In.ClipPosition.w) * SoftParticleDepthScale));
    
    
    

    
    
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
        zwriteenable = false;
        cullmode = none;
        fillmode = solid;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
};