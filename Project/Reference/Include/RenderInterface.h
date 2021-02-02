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
			NoAlpha,
			AlphaTest,
			AlphaBlend,
			Particle,
			DebugCollision,
			UI,
		};
	public:
		//   �ʱ�ȭ �ܰ迡�� ������ �Ѱ��ּ���.
		void SetUpCullingInformation(const Sphere CullingLocalSphere,
							class Transform*const RenderObjectTransform)&;
		void SetUpRenderingInformation(const Group _Group);
		//   �������� ���ϴ� Ÿ�̹��� ������Ʈ �����ӿ��� ȣ�����ּ���.
		void Regist();
		virtual void Render()& abstract;
		inline const Group GetGroup()const& { return _Group; };
		Sphere GetCullingSphere() const&; 
		bool bCullingOn{ true };
	private: 
		class Transform* RenderObjectTransform{ nullptr };
		const float* CullingSphereScale{ nullptr }; 
		const Matrix* CullingWorldMatrix{ nullptr };
		Sphere CullingLocalSphere{}; 
		Group _Group;
	};
};


