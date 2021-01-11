#pragma once
#include "DllHelper.h"

namespace Engine
{
	using uint32_t = unsigned long;

	class Component abstract
	{
	public:
		enum Property : uint32_t
		{
			// 컴포넌트 업데이트 순서에 영향을 끼침.
			Transform = 0u,
			Collision,
			Standard,
			Render,
			End,
		};
	public:
		virtual ~Component()noexcept = default;
	public:
		virtual void Update(class Object* const Owner,
			const float DeltaTime)& abstract;
	public:
		// 자식마다 정의.
		static const inline Property TypeProperty = Property::Standard;
	};
};



