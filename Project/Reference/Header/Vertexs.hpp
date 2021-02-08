#pragma once
#include "TypeAlias.h"
#include "AssimpHelper.h"
#include "FMath.hpp"

namespace Vertex
{
	struct Skeleton
	{
		Vector3 Location{ 0,0,0 };
		Vector3 Normal{ 0 , 0,  0 };
		Vector2 UV{ 0,0 };
		static inline Skeleton MakeFromAssimpMesh(const aiMesh* const AiMesh,const uint32 CurrentIdx)
		{
			return Skeleton
			{
				FromAssimp(AiMesh->mVertices[CurrentIdx]),
				FromAssimp(AiMesh->mNormals[CurrentIdx]),
				FMath::ToVec2(FromAssimp(AiMesh->mTextureCoords[0][CurrentIdx]))
			};
		};
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	};

	struct LocationNormalUV2D
	{
		Vector3 Location{ 0,0,0 };
		Vector3 Normal{ 0 , 0,  0 };
		Vector2 UV{ 0,0 };
		static inline LocationNormalUV2D MakeFromAssimpMesh(const aiMesh* const AiMesh,
			const uint32 CurrentIdx)
		{
			return LocationNormalUV2D
			{
				FromAssimp(AiMesh->mVertices[CurrentIdx]),
				FromAssimp(AiMesh->mNormals[CurrentIdx]),
				FMath::ToVec2(FromAssimp(AiMesh->mTextureCoords[0][CurrentIdx]))
			};
		};
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	};

	struct LocationUV2D
	{
		Vector3 Location{ 0,0,0 };
		Vector2 UV{ 0,0 };
		static inline LocationUV2D MakeFromAssimpMesh(const aiMesh* const AiMesh,
			const uint32 CurrentIdx)
		{
			return LocationUV2D
			{
				FromAssimp(AiMesh->mVertices[CurrentIdx]),
				FMath::ToVec2(FromAssimp(AiMesh->mTextureCoords[0][CurrentIdx]))
			};
		};
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_TEX1;
	};

	struct LocationNormal
	{
		Vector3 Location{ 0,0,0 };
		Vector3 Normal{ 0 , 0,  0 };
		static inline LocationNormal MakeFromAssimpMesh(const aiMesh* const AiMesh,
			const uint32 CurrentIdx)
		{
			return LocationNormal
			{
				FromAssimp(AiMesh->mVertices[CurrentIdx]),
				FromAssimp(AiMesh->mNormals[CurrentIdx]),
			};
		};
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL;
	};

	struct LocationColor
	{
		Vector3 Location{ 0,0,0 };
		D3DCOLOR Diffuse = D3DCOLOR_ARGB(255,165,171,255);
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	};


	struct Animation
	{
		Vector3 Location{}; 
		Vector3 Normal{}; 
		Vector2 UV{}; 
		Vector4 BoneIds{0,0,0,0};
		Vector4 BoneWeights{0,0,0,0};
		static IDirect3DVertexDeclaration9* const
			GetVertexDecl(IDirect3DDevice9* const Device);
	};

	struct Location3DUV
	{
		Vector3 Location;
		Vector3 UV;
		static const uint32 FVF = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(0);
	};
	struct Texture
	{
		Vector3 Location;
		Vector3 Normal;
		Vector2 TexCoord;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
		static IDirect3DVertexDeclaration9* const
			GetVertexDecl(IDirect3DDevice9* const Device);
	};

	struct Default
	{
		Vector3 Location;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL;
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

inline IDirect3DVertexDeclaration9* const
Vertex::Animation::GetVertexDecl(IDirect3DDevice9* const Device)
{
	D3DVERTEXELEMENT9 Decl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		D3DDECL_END()
	};
	IDirect3DVertexDeclaration9* VertexDeclaration{ nullptr };
	Device->CreateVertexDeclaration(Decl, &VertexDeclaration);
	return VertexDeclaration;
};

inline IDirect3DVertexDeclaration9* const Vertex::Texture::GetVertexDecl(IDirect3DDevice9* const Device)
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

inline IDirect3DVertexDeclaration9* const Vertex::TextureTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}

inline IDirect3DVertexDeclaration9* const Vertex::OnlyLocationTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}

inline IDirect3DVertexDeclaration9* const Vertex::Location3DUVTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}

inline IDirect3DVertexDeclaration9* const Vertex::Location2DUVTangent::GetVertexDecl(IDirect3DDevice9* const Device)
{
	return nullptr;
}
