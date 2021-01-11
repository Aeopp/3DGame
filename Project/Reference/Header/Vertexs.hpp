#pragma once
#include "TypeAlias.h"

namespace Vertex
{
	struct Texture
	{
		Vector3 Location;
		Vector3 Normal;
		Vector2 TexCoord;
		static IDirect3DVertexDeclaration9* const
			GetVertexDecl(IDirect3DDevice9* const Device);
	};

	struct TextureTangent
	{
		Vector3 Location;
		Vector3 Normal;
		Vector3 Tangent;
		Vector3 BiNormal;
		Vector2 TexCoord;
		static IDirect3DVertexDeclaration9* const
			GetVertexDecl(IDirect3DDevice9* const Device);
	};

	struct OnlyLocationTangent
	{
		Vector3 Location;
		Vector3 Normal;
		Vector3 Tangent;
		Vector3 BiNormal;
		Vector2 TexCoord;
		static IDirect3DVertexDeclaration9* const
			GetVertexDecl(IDirect3DDevice9* const Device);
	};

	struct Location3DUVTangent
	{
		Vector3 Location;
		Vector3 Normal;
		Vector3 Tangent;
		Vector3 BiNormal;
		Vector2 TexCoord;
		static IDirect3DVertexDeclaration9* const
			GetVertexDecl(IDirect3DDevice9* const Device);
	};

	struct Location2DUVTangent
	{
		Vector3 Location;
		Vector3 Normal;
		Vector3 Tangent;
		Vector3 BiNormal;
		Vector2 TexCoord;
		static IDirect3DVertexDeclaration9* const
			GetVertexDecl(IDirect3DDevice9* const Device);
	};
};

namespace Index
{
	struct _16
	{
		uint16 _1, _2, _3;
		static inline constexpr D3DFORMAT Format = D3DFMT_INDEX16;
	};
};


// Implementation

IDirect3DVertexDeclaration9* const Vertex::Texture::GetVertexDecl(IDirect3DDevice9* const Device)
{
	D3DVERTEXELEMENT9 Decl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	IDirect3DVertexDeclaration9* VertexDeclaration{ nullptr };
	Device->CreateVertexDeclaration(Decl, &VertexDeclaration);
	return VertexDeclaration;
}

IDirect3DVertexDeclaration9* const Vertex::TextureTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}

IDirect3DVertexDeclaration9* const Vertex::OnlyLocationTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}

IDirect3DVertexDeclaration9* const Vertex::Location3DUVTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}

IDirect3DVertexDeclaration9* const Vertex::Location2DUVTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}