matrix World;
matrix View;
matrix Projection;

float AlphaFactor = 1.f;
float Brightness = 1.f;
texture DiffuseMap;
// VTF 텍스쳐
int VTFPitch;
texture VTF;
texture PatternMap;

sampler VTFSampler = sampler_state
{
    texture = VTF;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
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
    float4 BoneIds : BLENDINDICES;
    float4 Weights : BLENDWEIGHT;
};

struct VS_OUT
{
    vector Position : POSITION;
    float2 UV : TEXCOORD0;
};

// 정점 쉐이더
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    float4 AnimNormal = float4(0, 0, 0, 0);
    float4 AnimPos = float4(0, 0, 0, 1);
    
    In.Position.w = 1.0f;
    
    float UVCorrection = 0.5f;
    float FVTFPitch = float(VTFPitch);
    int   IVTFPitch = int(VTFPitch);
    
    for (int i = 0; i < 4; ++i)
    {
        int Idx = In.BoneIds[i] * 4;
        
        float2 VTFUVRow0 = float2((float(Idx % IVTFPitch) + UVCorrection) / FVTFPitch,
                                  (float(Idx / IVTFPitch) + UVCorrection) / FVTFPitch);
        
        float2 VTFUVRow1 = float2((float((Idx + 1) % IVTFPitch) + UVCorrection) / FVTFPitch,
                                  (float((Idx + 1) / IVTFPitch) + UVCorrection) / FVTFPitch);
        
        float2 VTFUVRow2 = float2((float((Idx + 2) % IVTFPitch) + UVCorrection) / FVTFPitch,
                                  (float((Idx + 2) / IVTFPitch) + UVCorrection) / FVTFPitch);
        
        float2 VTFUVRow3 = float2((float((Idx + 3) % IVTFPitch) + UVCorrection) / FVTFPitch,
                                  (float((Idx + 3) / IVTFPitch) + UVCorrection) / FVTFPitch);
        
        float4x4 AnimMatrix =
        {
            tex2Dlod(VTFSampler, float4(VTFUVRow0, 0.f, 0.f)),
            tex2Dlod(VTFSampler, float4(VTFUVRow1, 0.f, 0.f)),
            tex2Dlod(VTFSampler, float4(VTFUVRow2, 0.f, 0.f)),
            tex2Dlod(VTFSampler, float4(VTFUVRow3, 0.f, 0.f))
        };
        
        AnimNormal += (mul(float4(In.Normal, 0.f), AnimMatrix) * In.Weights[i]);
        AnimPos += (mul(In.Position, AnimMatrix) * In.Weights[i]);
    }
    matrix WorldView, WorldViewProjection;

    WorldView = mul(World, View);
    WorldViewProjection = mul(WorldView, Projection);

    Out.Position = mul(vector(AnimPos.xyz, 1.f), WorldViewProjection);
    Out.UV = In.UV;
//    Out.Normal = mul(float4(AnimNormal.xyz, 0.f), World);
    
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
    
    float4 DiffuseColor = tex2D(DiffuseSampler, In.UV);
    float4 Pattern = tex2D(PatternSampler, In.UV);
    Out.Color = DiffuseColor * Pattern;
    
    Out.Color.rgb *= Brightness;
    Out.Color.a *= AlphaFactor;
    
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
        StencilEnable = true;
        StencilFunc = always;
        StencilPass = replace;
        StencilRef = 1;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}