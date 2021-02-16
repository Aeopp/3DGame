#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Object.h"
#include <filesystem>
#include <string>
#include "Frustum.h"
#include "FMath.hpp"
#include "ShaderFx.h"
#include <unordered_map>
#include "MaterialInformation.h"


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
			DWORD  FVF{ 0u };
			std::string Name{};
			Engine::MaterialInformation MaterialInfo{};
			Sphere BoundingSphere{};
		};
		struct DecoInformation
		{
			bool bLandscapeInclude = false;
			bool bPendingKill = false;
			Vector3 Scale{ 1,1,1 };
			Vector3 Rotation{ 0,0,0 };
			Vector3 Location{ 0,0,0 };
		};
		struct Decorator
		{
			std::vector<std::shared_ptr<DecoInformation>> Instances{};
			std::vector<Mesh> Meshes{};
		};
	public :
		void Initialize(IDirect3DDevice9* const Device,
			const Vector3 Scale,
			const Vector3 Rotation,
			const Vector3 Location,
			const std::filesystem::path FilePath,
			const std::filesystem::path FileName
		)&;

		void Render(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection ,const Vector3& CameraLocation)&;
		inline const std::vector<PlaneInfo>& GetMapWorldCoordPlanes()const&;

		void DecoratorLoad(const std::filesystem::path& LoadPath,
							const std::filesystem::path& LoadFileName)&;

		std::weak_ptr<typename Engine::Landscape::DecoInformation>
			PushDecorator(const std::wstring DecoratorKey , 
						    const Vector3& Scale , const Vector3& Rotation, const Vector3& Location,
							const bool bLandscapePolygonInclude)&; 

		typename Engine::Landscape::Decorator*
			GetDecorator(const std::wstring DecoratorKey)&;

		std::weak_ptr<DecoInformation> 
			PickDecoInstance(const Ray WorldRay)&;

		bool bDecoratorSphereMeshRender{ false }; 

		void Save(const std::filesystem::path& SavePath)&; 
		void Load(const std::filesystem::path& LoadPath)&;

		inline const std::string& GetDecoratorSaveInfo()& { return DecoratorSaveInfo; };
	private:
		std::string DecoratorSaveInfo{}; 
		IDirect3DVertexDeclaration9* VtxDecl{ nullptr };
		Engine::ShaderFx _ShaderFx{};
		Engine::ShaderFx _ShaderFxNonCavity{};
		Vector3 Scale{1,1,1};
		Vector3 Rotation{0,0,0};
		Vector3 Location{0,0,0}; 
		IDirect3DDevice9*      Device{ nullptr };
		std::vector<PlaneInfo> WorldPlanes{};
		std::vector<Mesh>      Meshes{};
		std::unordered_map<std::wstring,Decorator> DecoratorContainer{};
	};
};

inline const std::vector<PlaneInfo>& Engine::Landscape::GetMapWorldCoordPlanes()const&
{
	return WorldPlanes;
}






