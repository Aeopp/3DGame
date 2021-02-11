matrix  World;
matrix  View;
matrix  Projection;

struct VS_IN
{
	vector			Position : POSITION;
	vector          Color    : TEXCOORD0;
};

struct VS_OUT
{
	vector Position : POSITION;
	vector Color : TEXCOORD0;
};

// ¡§¡° Ω¶¿Ã¥ı
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out = (VS_OUT)0;

	matrix		WorldView, WorldViewProjection;

	WorldView = mul(World, View);
	WorldViewProjection = mul(WorldView, Projection);

	Out.Position =Out.Position = mul(vector(In.Position.xyz, 1.f), WorldViewProjection);
	Out.Color = In.Color;

	return Out;
}


struct PS_IN
{
	vector Color : TEXCOORD0;
};

struct PS_OUT
{
	vector			Color : COLOR0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.Color = In.Color;

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
};