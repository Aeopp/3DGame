#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Object.h"
#include <filesystem>
#include "Frustum.h"
#include "FMath.hpp"
#include "ShaderFx.h"
#include <unordered_map>


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
			IDirect3DTexture9* DiffuseMap{ nullptr };
			IDirect3DTexture9* SpecularMap{ nullptr };
			IDirect3DTexture9* NormalMap{ nullptr };
			IDirect3DTexture9* EmissiveMap{ nullptr };
			Sphere BoundingSphere{};
			float RimWidth = 0.8f; 
			float Power = 64.f;
			Vector4 RimAmtColor { 1,1,1,1 }; 
			Vector4 AmbientColor{ 0.15f,0.15f,0.15f,1.f };
		};
		struct Decorator
		{
			std::vector<std::tuple<float, Vector3, Vector3>> Transforms{}; 
			std::vector<Mesh> Meshes{};
		};
	public :
		void Initialize(IDirect3DDevice9* const Device,
			const Matrix& MapWorld,
			const std::filesystem::path FilePath,
			const std::filesystem::path FileName
		)&;

		void Render(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection ,const Vector3& CameraLocation)&;
		inline std::vector<PlaneInfo> GetMapWorldCoordPlanes()const&;

		void  DecoratorLoad(const std::filesystem::path& LoadPath,
							const std::filesystem::path& LoadFileName)&;
		void PushDecorator(const std::wstring DecoratorKey , 
						const float Scale , const Vector3& Rotation, const Vector3& Location)&; 

		bool bDecoratorSphereMeshRender{ true }; 
	private:
		IDirect3DVertexDeclaration9* VtxDecl{ nullptr };
		Engine::ShaderFx _ShaderFx{};
		Matrix World = FMath::Identity();
		IDirect3DDevice9*      Device{ nullptr };
		std::vector<Vector3>   WorldVertexLocation{};
		std::vector<PlaneInfo> WorldPlanes{};
		std::vector<Mesh>      Meshes{};
		std::unordered_map<std::wstring,Decorator> DecoratorContainer{};
	};
};

inline std::vector<PlaneInfo> Engine::Landscape::GetMapWorldCoordPlanes()const&
{
	return WorldPlanes;
}






