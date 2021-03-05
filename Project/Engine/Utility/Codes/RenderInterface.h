#pragma once
#include "DllHelper.H"
#include "TypeAlias.h"
#include "FMath.hpp"

namespace Engine
{
	class DLL_DECL RenderInterface abstract
	{
	public:
		enum class Group :uint8
		{
			Enviroment = 0u,
			DeferredNoAlpha,  
			NoAlpha,
			AlphaTest,
			AlphaBlend,
			Particle,
			DebugCollision,
			UI,
		};
	public:
		//   초기화 단계에서 정보를 넘겨주세요.
		void SetUpCullingInformation(const Sphere CullingLocalSphere,
							class Transform*const RenderObjectTransform)&;
		void SetUpRenderingInformation(const Group _Group);
		//   렌더링을 원하는 타이밍의 업데이트 루프속에서 호출해주세요.
		void Regist();
		virtual void Render(const Matrix& View, const Matrix& Projection,
							const Vector4& CameraLocation)& abstract;

		virtual void RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(
			Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation)&;
		virtual void RenderShadowDepth(
			const Matrix& LightViewProjection)&;
		virtual void RenderDeferredAfter(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation,
			IDirect3DTexture9* const ShadowDepthMap,
			const Matrix& LightViewProjection,
			const float ShadowDepthMapSize,
			const float ShadowDepthBias,
			const Vector3& FogColor,
			const float FogDistance)&;

		inline const Group GetGroup()const& { return _Group; };
		Sphere GetCullingSphere() const&; 
		bool bCullingOn{ true };
	protected:
		Group _Group;
	private: 
		class Transform* RenderObjectTransform{ nullptr };
		const float* CullingSphereScale{ nullptr }; 
		const Matrix* CullingWorldMatrix{ nullptr };
		Sphere CullingLocalSphere{}; 
	};
};


