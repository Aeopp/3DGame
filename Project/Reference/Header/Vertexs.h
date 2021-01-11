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






// Implementation

IDirect3DVertexDeclaration9* const Vertex::Texture::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
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
