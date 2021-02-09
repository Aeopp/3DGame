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
	float2			UV : TEXCOORD0;
};

struct VS_OUT
{
	vector Position : POSITION;
	float4 Location : TEXCOORD0;
	float2 UV : TEXCOORD1;
};

// 정점 쉐이더
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out = (VS_OUT)0;

	matrix		WorldView, WorldViewProjection;

	WorldView = mul(World, View);
	WorldViewProjection = mul(WorldView, Projection);

	Out.Location=Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
	Out.UV= In.UV;

	return Out;
}

//VS_OUT VS_TEMP(VS_IN In)
//{
//	VS_OUT Out = (VS_OUT)0;
//
//	matrix		matWV, matWVP;
//
//	matWV = mul(g_matWorld, g_matView);
//	matWVP = mul(matWV, g_matProj);
//
//	Out.vPosition = mul(vector(In.vPosition.xyz, 1.f), matWVP);
//	Out.vTexUV = In.vTexUV;
//
//	return Out;
//}

struct PS_IN
{
	float4    Location : TEXCOORD0;
	float2			UV : TEXCOORD1;
};

struct PS_OUT
{
	vector			Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.Color = tex2D(BaseSampler, In.UV); 
	// 2차원 텍스쳐의 UV정보로부터 해당하는 픽셀의 색상을 추출하는 함수, 반환 타입은 vector
	Out.Color.a = 1.0f;
	
	In.Location.xyzw = (In.Location.xyzw / In.Location.w);
	
	if(In.Location.x <0.f)
	{
		Out.Color.r *= 2.f;
	}
	else
	{
		Out.Color.g *= 2.f;
	}
	
	return Out;
}

technique Default_Device
{
	// 기능의 캡슐화
	pass	
	{
		alphablendenable = true;
		srcblend = srcalpha;
		destblend = invsrcalpha;

		vertexshader = compile vs_3_0 VS_MAIN();
		pixelshader = compile ps_3_0 PS_MAIN();
	}

	/*pass
	{
		alphaTestenable = true;
		alphafunc = greater;
		alpharef = 0xc0;

		vertexshader = compile vs_3_0 VS_TEMP();
		pixelshader = compile ps_3_0 PS_MAIN();
	}*/
};