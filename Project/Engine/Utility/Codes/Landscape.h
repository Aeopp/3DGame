#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Object.h"
#include <filesystem>
#include "Frustum.h"
#include "FMath.hpp"

namespace Engine
{
	class DLL_DECL Landscape
	{
	public:
		struct Mesh
		{
			IDirect3DVertexBuffer9* VtxBuf{ nullptr };
			IDirect3DIndexBuffer9* IdxBuf{ nullptr };
			uint32 VtxCount{ 0u };
			uint32 Stride{ 0u };
			uint32 PrimitiveCount{ 0u };
			DWORD FVF{ 0u };
			IDirect3DTexture9* DiffuseMap{nullptr};
			IDirect3DTexture9* SpecularMap{ nullptr };
			IDirect3DTexture9* NormalMap{ nullptr };
		};
	public :
		void Initialize(IDirect3DDevice9* const Device,
			const Matrix& MapWorld,
			const std::filesystem::path FilePath,
			const std::filesystem::path FileName
		) & ;
		void Render(Engine::Frustum& RefFrustum)&;
	private:
		Matrix World = FMath::Identity();
		IDirect3DDevice9* Device{ nullptr };
		std::vector<Vector3>LocalVertexLocations{};
		std::vector<Mesh> Meshes{};
	};
};



