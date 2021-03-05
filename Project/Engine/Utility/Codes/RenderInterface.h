#pragma once
#include "DllHelper.H"
#include "TypeAlias.h"
#include "FMath.hpp"
#include "Frustum.h"



namespace Engine
{
	class Renderer;

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
		bool FrustumInCheck(Engine::Frustum& RefFrustum)&;
		//   초기화 단계에서 정보를 넘겨주세요.
		void SetUpCullingInformation(const Sphere CullingLocalSphere,
							class Transform*const RenderObjectTransform)&;
		void SetUpRenderingInformation(const Group _Group);
		//   렌더링을 원하는 타이밍의 업데이트 루프속에서 호출해주세요.
		void Regist();

		// 기본 쉐이더 . (포워드)
		virtual void Render(Engine::Renderer* const _Renderer)&abstract;
		// 지연 패스에 필요한 노말 알베도등 속성을 렌더타겟에 렌더링. 
		virtual void RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Renderer* const _Renderer)& ;
		// 쉐도우 맵에 필요한 광원을 시점으로한 깊이정보를 렌더링. 
		virtual void RenderShadowDepth(Engine::Renderer* const _Renderer)&;
		virtual void RenderReady(Engine::Renderer* const _Renderer)&;

		inline const Group GetGroup()const& { return _Group; };
		Sphere GetCullingSphere() const&; 
		bool bCullingOn{ true };
		bool bCurrentFrustumIn{ true };
	protected:
		Group _Group;
	private: 
		class Transform* RenderObjectTransform{ nullptr };
		const float*     CullingSphereScale{ nullptr }; 
		const Matrix*    CullingWorldMatrix{ nullptr };
		Sphere           CullingLocalSphere{}; 
	};
};


