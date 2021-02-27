#pragma once
#include <array>
#include "TypeAlias.h"
#include "MathStruct.h"
#include "DllHelper.H"
#include <filesystem>
#include "ShaderFx.h"
#include "Frustum.h"

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
			IDirect3DVertexDeclaration9* VtxDecl{ nullptr }; 
			IDirect3DTexture9*					  DiffuseTexture{ nullptr };
			uint32 Stride{ 0u }; 
		};
		void Initialize(const std::filesystem::path& FullPath , IDirect3DDevice9* const Device)&;
		void Render(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection,
			const Vector4& CameraLocation4D,
			IDirect3DDevice9* const Device,
			IDirect3DTexture9* DepthTexture)&;
	private:
		float RotationAcc{ 0.03f };
		float Rotation{ 0.f };
		SkyMesh _SkyMesh{}; 		

		Engine::ShaderFx SkyShaderFx{};
	};
};
