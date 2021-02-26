#pragma once
#include <array>
#include "TypeAlias.h"
#include "MathStruct.h"
#include "DllHelper.H"
#include <filesystem>

namespace Engine
{
	class DLL_DECL Sky
	{
	public :
		struct SkyMesh 
		{
			std::vector<IDirect3DVertexBuffer9*>  VtxBufs{}; 
			std::vector<IDirect3DIndexBuffer9*>   IdxBufs{ nullptr };
			std::vector<uint32> VertexCounts{};
			std::vector<uint32> PrimCount{}; 
			IDirect3DTexture9*					  Texture{ nullptr };
			uint32 Stride{ 0u }; 
		};
		void Initialize(const std::filesystem::path& FullPath , IDirect3DDevice9* const Device)&;
		void Render(const Vector3& CameraLocation , IDirect3DDevice9* const Device)&;
	private:
		float RotationAcc{ 1.f };
		float Rotation{ 0.f };
		SkyMesh _SkyMesh{}; 		
	};
};
