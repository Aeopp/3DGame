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
		//   �ʱ�ȭ �ܰ迡�� ������ �Ѱ��ּ���.
		void SetUpCullingInformation(const Sphere CullingLocalSphere,
							class Transform*const RenderObjectTransform)&;
		void SetUpRenderingInformation(const Group _Group);
		//   �������� ���ϴ� Ÿ�̹��� ������Ʈ �����ӿ��� ȣ�����ּ���.
		void Regist();

		// �⺻ ���̴� . (������)
		virtual void Render(Engine::Renderer* const _Renderer)&abstract;
		// ���� �н��� �ʿ��� �븻 �˺����� �Ӽ��� ����Ÿ�ٿ� ������. 
		virtual void RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Renderer* const _Renderer)& ;
		// ������ �ʿ� �ʿ��� ������ ���������� ���������� ������. 
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


