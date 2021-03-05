#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Object.h"
#include <filesystem>
#include <set>
#include <string>
#include "Frustum.h"
#include "FMath.hpp"
#include "ShaderFx.h"
#include <unordered_map>
#include "MaterialInformation.h"
#include <optional>
#include <any>

namespace Engine
{
	class DLL_DECL Landscape
	{
	public:
		struct Mesh
		{
			Mesh(); 
			Mesh(const Mesh&) = default;
			Mesh(Mesh&&)noexcept = default;
			Mesh&operator=(Mesh&&)noexcept = default;
			Mesh&operator=(const Mesh&) = default;
			IDirect3DVertexBuffer9* VtxBuf{ nullptr };
			IDirect3DIndexBuffer9* IdxBuf{ nullptr };
			uint32 ID = 0u;
			uint32 VtxCount{ 0u };
			uint32 Stride{ 0u };
			uint32 PrimitiveCount{ 0u };
			DWORD  FVF{ 0u };
			std::string Name{};
			Engine::MaterialInformation MaterialInfo{};
			Sphere BoundingSphere{};
		};
		struct FloatingInformation
		{
			static inline std::pair<float, float> VibrationWidthRange{ 0.f,10.f };
			static inline std::pair<float, float > RotationAccRange  { 0.0f,0.00003f };
			static inline std::pair<float, float>  VibrationAccRange  { 0.0f,1.f };
			
			static void RangeEdit();
			void Initialize()&
			{
				VibrationWidth=FMath::Random(VibrationWidthRange.first, VibrationWidthRange.second);
				RotationAcc = FMath::Random(RotationAccRange.first, RotationAccRange.second);
				VibrationAcc = FMath::Random(VibrationAccRange.first, VibrationAccRange.second);
				RotationFactorSign = static_cast <float>(FMath::Random(0u, 1u));
				RotationFactorSign = RotationFactorSign * 2.f - 1.f;
				VibrationT = 0.0f;
				Radian = 0.0f;
			}
			// 부유하는 물체들 운동시킨 이후의 위치를 계산합니다. 
			Vector3 Floating(const float DeltaTime, 
				const Vector3& BeforeApplyingLocation /*떠다니는 운동 적용 전 기준 위치를 넘겨주세요.*/)&
			{
				VibrationT += (DeltaTime * VibrationAcc);
				Radian += (RotationAcc * DeltaTime) * RotationFactorSign;
				Vector3 ApplyFloatingLocation = FMath::RotationVecCoord(BeforeApplyingLocation, { 0,1,0 }, Radian);
				ApplyFloatingLocation.y += std::sinf(VibrationT) * VibrationWidth;
				return ApplyFloatingLocation;
			}
			// 상하 운동
			float VibrationT{ 0.0f };
			float VibrationWidth{ 1.f };
			float VibrationAcc{ 1.f };
			// 자전 월드 원점 기준 Yaw
			float Radian{ 0.0f };
			float RotationAcc{ 1.f };
			float RotationFactorSign{ 1.f };
		};
		struct DecoInformation
		{
			bool bLandscapeInclude = false;
			bool bPendingKill = false;
			Vector3 Scale{ 1,1,1 };
			Vector3 Rotation{ 0,0,0 };
			Vector3 Location{ 0,0,0 };
			std::set<uint32> CurRenderIDSet{};
			std::any OptionValue{}; 
		};
		struct Decorator
		{
			enum class Option :uint8
			{
				None,
				Floating,
			};
			std::vector<std::shared_ptr<DecoInformation>> Instances{};
			std::vector<Mesh> Meshes{};
			Option _Option{ Option::None };
		};
	public :
		void Initialize(IDirect3DDevice9* const Device,
			const Vector3 Scale,
			const Vector3 Rotation,
			const Vector3 Location)&;

		void Tick(const float Tick)&;

		void Render(Engine::Frustum& RefFrustum,
					const Matrix& View, const Matrix& Projection ,const Vector4& CameraLocation,
					IDirect3DTexture9* const ShadowDepthMap ,
					const Matrix& LightViewProjection,
					const float ShadowDepthMapWidth,
					const float ShadowDepthMapHeight,
					const float ShadowDepthBias ,
					const Vector3& FogColor,
					const float FogDistance)&;

		void FrustumCullingCheck(Engine::Frustum& RefFrustum)&;

		void RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation)&;

		void RenderShadowDepth(
			const Matrix& LightViewProjection)&;

		inline const std::vector<PlaneInfo>& GetMapWorldCoordPlanes()const&;

		void DecoratorLoad(const std::filesystem::path& LoadPath,
							const std::filesystem::path& LoadFileName)&;

		std::pair< std::weak_ptr<typename Engine::Landscape::DecoInformation>, std::wstring >
			PushDecorator(const std::wstring DecoratorKey , 
						    const Vector3& Scale , const Vector3& Rotation, const Vector3& Location,
							const bool bLandscapePolygonInclude)&; 

		std::pair< std::weak_ptr<typename Engine::Landscape::DecoInformation>, std::wstring >
			PushDecorator(const std::wstring DecoratorKey,
				const Vector3& Scale, const Vector3& Rotation, const Vector3& Location,
				const bool bLandscapePolygonInclude , const Ray WorldRay)&;

		typename Engine::Landscape::Decorator*
			GetDecorator(const std::wstring DecoratorKey)&;

		std::pair<std::weak_ptr<typename Engine::Landscape::DecoInformation>, std::wstring>
			PickDecoInstance(const Ray WorldRay)&;

		std::optional<Vector3 > RayIntersectPoint(const Ray  WorldRay)const&;

		bool bDecoratorSphereMeshRender{ false }; 

		void Save(const std::filesystem::path& SavePath)&; 
		void Load(const std::filesystem::path& LoadPath)&;
		void Clear()&;

		inline const std::string& GetDecoratorSaveInfo()& { return DecoratorSaveInfo; };

		// 지형으로 인식되는 폴리곤 정보를 삭제후 다시 계산합니다 주로 지형으로 인식되던 스태틱 메시를 삭제 했을경우 유용.
		void ReInitWorldPlanes()&;

		void FloatingDecoInstancesReInit()&;
	private:
		std::string DecoratorSaveInfo{}; 
		IDirect3DVertexDeclaration9* VtxDecl{ nullptr };
		Engine::ShaderFx ForwardShaderFx{};
		Engine::ShaderFx DeferredAlbedoNormalWorldPosDepthSpecular{};
		
		Engine::ShaderFx ShadowDepthFx{};
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






