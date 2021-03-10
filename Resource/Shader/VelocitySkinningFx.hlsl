matrix WorldView;
matrix PrevWorldView;

matrix Projection;


int VTFPitch;
texture VTF;
texture PrevVTF;

sampler VTFSampler = sampler_state
{
    texture = VTF;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};


sampler PrevVTFSampler = sampler_state
{
    texture = PrevVTF;

    minfilter = point;
    magfilter = point;
    mipfilter = point;
};

struct VS_IN
{
    vector Position : POSITION;
    float3 Normal  : NORMAL;
    float4 BoneIds : BLENDINDICES;
    float4 Weights : BLENDWEIGHT;
};

struct VS_OUT
{
    vector Position : POSITION;
    vector Velocity : TEXCOORD0;
};

// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    In.Position.w = 1.0f;
    
    
    float UVCorrection = 0.5f;
    float FVTFPitch = float(VTFPitch);
    int   IVTFPitch = int(VTFPitch);
    
    float4 CurAnimPos = float4(0, 0, 0, 1);
    float4 PrevAnimPos = float4(0, 0, 0, 1);
    float3 AnimNormal = float3(0, 0, 0);
    
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
        
        float4x4 PrevAnimMatrix =
        {
            tex2Dlod(PrevVTFSampler, float4(VTFUVRow0, 0.f, 0.f)),
            tex2Dlod(PrevVTFSampler, float4(VTFUVRow1, 0.f, 0.f)),
            tex2Dlod(PrevVTFSampler, float4(VTFUVRow2, 0.f, 0.f)),
            tex2Dlod(PrevVTFSampler, float4(VTFUVRow3, 0.f, 0.f))
        };
        PrevAnimPos +=     (mul(In.Position, PrevAnimMatrix) * In.Weights[i]);
        CurAnimPos  +=     (mul(In.Position, AnimMatrix) * In.Weights[i]);
        AnimNormal  +=     (mul(float4(In.Normal, 0.f), AnimMatrix) * In.Weights[i]);
    }
    
    
    
    
    
    float4 CurPosition = 
      mul(float4(CurAnimPos.xyz, 1.f), WorldView);
    
    float4 PrevPosition = 
        mul(float4(PrevAnimPos.xyz, 1.f), PrevWorldView);
    
    float4 Direction = CurPosition - PrevPosition;
    float3 Normal = mul(float4(AnimNormal, 0.0f), WorldView);
    Normal = normalize(Normal);
    
    bool bFront = (0 <= dot(Normal, Direction.xyz));
    float4 SelectPosition = bFront ? CurPosition : PrevPosition;
    
    Out.Position = mul(SelectPosition, Projection);
    
    CurPosition = mul(CurPosition, Projection);
    PrevPosition = mul(PrevPosition, Projection);
    
    float2 Velocity = (CurPosition.xy / CurPosition.w)
                    - (PrevPosition.xy / PrevPosition.w);
    
    Out.Velocity.xy = Velocity * 0.5f;
    Out.Velocity.y *= -1.f;
    Out.Velocity.z = Out.Position.z;
    Out.Velocity.w = Out.Position.w;
    
    return Out;
}


struct PS_IN
{
    vector Velocity : TEXCOORD0;
};

struct PS_OUT
{
    vector Velocity : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.Velocity.xy = In.Velocity.xy;
    Out.Velocity.z = 1.0f;
    Out.Velocity.w = In.Velocity.z / In.Velocity.w;
    Out.Velocity.xy *= 1.5f;
    
    return Out;
}

technique Default_Device
{
    pass
    {
        alphablendenable = false;
        zenable = true;
        zwriteenable = false;
        cullmode = ccw;
        fillmode = solid;
        StencilEnable = false;
        vertexshader = compile vs_3_0 VS_MAIN();
        pixelshader = compile ps_3_0 PS_MAIN();
    }
}