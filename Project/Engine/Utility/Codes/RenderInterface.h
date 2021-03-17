#pragma once
#include "DllHelper.H"
#include "TypeAlias.h"
#include "FMath.hpp"
#include "Frustum.h"
#include "ShaderFx.h"
#include <optional>

namespace Engine
{
	class Renderer;

	class DLL_DECL RenderInterface abstract
	{
	public:
		enum class Group :uint8
		{
			DeferredNoAlpha =  0u , 
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
		virtual void RenderDeferredAlbedoNormalVelocityDepthSpecularRim(Engine::Renderer* const _Renderer)& ;
		// ������ �ʿ� �ʿ��� ������ ���������� ���������� ������. 
		virtual void RenderShadowDepth(Engine::Renderer* const _Renderer)&;
		virtual void RenderReady(Engine::Renderer* const _Renderer)&;
		virtual void RenderVelocity(Engine::Renderer* const _Renderer)&;
		
		inline const Group GetGroup()const& { return _Group; };
		Sphere GetCullingSphere() const&; 
		bool bShadowDepth{ true };
		bool bCullingOn{ true };
		bool bCurrentFrustumIn{ true };
		bool bMotionBlur{ true };
		class Transform* RenderObjectTransform{ nullptr };

		
		struct DissolveInfo
		{
			float SliceAmount = 0.f;
			float BurnSize = 0.3f;
			float EmissionAmount = 3.f;
			 bool bBlueBurn = true;
		};
		std::optional<DissolveInfo> _DissolveInfo{};
	protected:
		Engine::ShaderFx VelocityFx{};
		Engine::ShaderFx ForwardShaderFx{};
		Engine::ShaderFx DepthShadowFx{};
		Engine::ShaderFx DeferredDefaultFx{};
		Engine::ShaderFx DeferredDissolveFx{};
		std::string DebugName{};
		Group _Group;
		
	private:
		const float*     CullingSphereScale{ nullptr }; 
		const Matrix*    CullingWorldMatrix{ nullptr };
		Sphere           CullingLocalSphere{}; 
	};
};


