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
		//   초기화 단계에서 정보를 넘겨주세요.
		void SetUpCullingInformation(const Sphere CullingLocalSphere,
							class Transform*const RenderObjectTransform)&;
		void SetUpRenderingInformation(const Group _Group);
		//   렌더링을 원하는 타이밍의 업데이트 루프속에서 호출해주세요.
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


