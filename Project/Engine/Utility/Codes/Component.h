#pragma once
#include "DllHelper.h"
#include "TypeAlias.h"

namespace Engine
{
	class DLL_DECL Component abstract
	{
	public:
		enum Property : uint32
		{
			// 컴포넌트 업데이트 순서에 영향을 끼침.
			Transform = 0u,
			Standard,
			Collision,
			Render,
			End,
		};
	public:
		virtual ~Component()noexcept = default;
		Component(Component&&)noexcept = default;
		Component(const Component&) = default;
		Component()=default;
	public:
		void Initialize()&;
		virtual void Update(class Object* const Owner,
			const float DeltaTime)& abstract;
	public:
		// 자식마다 정의.
		static const inline Property TypeProperty = Property::Standard;
	};
};



