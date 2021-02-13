matrix  World;
matrix  View;
matrix  Projection;

texture Diffuse;

sampler BaseSampler = sampler_state
{
    texture = Diffuse;

    minfilter = linear;
    magfilter = linear;
};

struct VS_IN
{
	vector			Position : POSITION;
    vector          Normal : NORMAL;
	float2          UV : TEXCOORD0;
};

struct VS_OUT
{
	vector Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
};

// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out = (VS_OUT)0;

	matrix		WorldView, WorldViewProjection;

	WorldView = mul(World, View);
	WorldViewProjection = mul(WorldView, Projection);

	Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
    Out.UV = In.UV;
    Out.Normal = mul( float4(In.Normal.xyz, 0.f), World);
    
	return Out;
}


struct PS_IN
{
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
};

struct PS_OUT
{
	vector			Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
    float3 Normal  = normalize(In.Normal);
    
    Out.Color = tex2D(BaseSampler, In.UV);
    
    Out.Color.rgb *= saturate(dot(float3(0, 1, 0), Normal));
    
    
    //Out.Color = float4(1.f, 0, 0, 1.f);
    
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