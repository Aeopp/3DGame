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
			// ������Ʈ ������Ʈ ������ ������ ��ħ.
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
		// �ڽĸ��� ����.
		static const inline Property TypeProperty = Property::Standard;
	};
};



