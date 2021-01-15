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
			// ������Ʈ ������Ʈ ������ ������ ��ħ.
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
		// �ڽĸ��� ����.
		static const inline Property TypeProperty = Property::Standard;
	};
};



