matrix  World;
matrix  View;
matrix  Projection;
float Power;
vector  Sun;
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
	float2			UV : TEXCOORD0;
};

struct VS_OUT
{
    vector Position : POSITION;
    float3 Normal : TEXCOORD0;
	float2 UV : TEXCOORD1;
    float3 ToSun : TEXCOORD2;
    float Distance : TEXCOORD3;
};

// ���� ���̴�
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out = (VS_OUT)0;

	matrix		WorldView, WorldViewProjection;

	WorldView = mul(World, View);
	WorldViewProjection = mul(WorldView, Projection);

	Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
	Out.UV= In.UV;
    Out.Normal = mul(vector(In.Normal.xyz, 0.f), WorldView).xyz;
	
    float3 WorldViewLocation = mul(vector(In.Position.xyz, 1.f), WorldView).xyz;
    float3 ViewSun = mul(vector(Sun.xyz, 1.f), View).xyz;
	
    float3 ToSun = WorldViewLocation - ViewSun;
    ToSun *= -1.0f;
    Out.ToSun = ToSun;
    Out.Distance = length(ToSun);
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
    float3 Normal : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 ToSun : TEXCOORD2;
    float Distance : TEXCOORD3;
};

struct PS_OUT
{
	vector			Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

    float3 Normal = normalize(In.Normal);
	Out.Color = tex2D(BaseSampler, In.UV); 
    float3 vToSun = normalize(In.ToSun);
    Out.Color*=dot(Normal, vToSun);
    Out.Color *= (Power /In.Distance);
	// 2���� �ؽ����� UV�����κ��� �ش��ϴ� �ȼ��� ������ �����ϴ� �Լ�, ��ȯ Ÿ���� vector
	
	return Out;
}

technique Default_Device
{
	// ����� ĸ��ȭ
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