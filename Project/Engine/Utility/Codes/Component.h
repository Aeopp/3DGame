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
			Standard=0u,
			Transform,
			Collision,
			Render,
			End,
		};
	public:
		virtual ~Component()noexcept = default;
	public:
		inline Property GetProperty()const& { return _Property; };
	public:
		virtual void Update(const float DeltaTime)&abstract;
	private:
		Property _Property;
	};
};



