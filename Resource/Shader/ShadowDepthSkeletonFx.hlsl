matrix World;
matrix LightViewProjection;

int VTFPitch;
texture VTF;
sampler VTFSampler = sampler_state
{
    texture = VTF;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};

struct VS_IN
{
    vector Position : POSITION;
    float4 BoneIds  : BLENDINDICES;
    float4 Weights  : BLENDWEIGHT;
};

struct VS_OUT
{
    vector Position : POSITION;
    vector ClipPosition : TEXCOORD0;
};

// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    float4 AnimPos = float4(0, 0, 0, 1);
    In.Position.w = 1.0f;
    
    float UVCorrection = 0.5f;
    float FVTFPitch = float(VTFPitch);
    int IVTFPitch = int(VTFPitch);
    
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
        
        AnimPos += (mul(In.Position, AnimMatrix) * In.Weights[i]);
    }
    
    matrix WVP = mul(World, LightViewProjection);
    
    Out.Position = mul(vector(AnimPos.xyz, 1.f), WVP);
    Out.ClipPosition = Out.Position;
    
    return Out;
}

struct PS_IN
{
    vector ClipPosition : TEXCOORD0;
};

struct PS_OUT
{
    vector ShadowDepth : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    In.ClipPosition /= In.ClipPosition.w;
	Out.ShadowDepth = In.ClipPosition.zzzz;
	
    return Out;
}




technique Default_Device
{
    pass
    {
        alphablendenable = false;
        zenable = true;
        zwriteenable = true;
        cullmode = ccw;
        fillmode = solid;
        stencilenable = false; 
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}