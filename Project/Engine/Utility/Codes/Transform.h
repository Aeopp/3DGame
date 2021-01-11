#pragma once
#include "Component.h"

namespace Engine
{
	class Transform : public  Component
	{
	public:
		using Super = Component;
	public:
		virtual void Update(class Engine::Object* const Owner, const float DeltaTime) & override;
	public:
		static const inline Property TypeProperty = Property::Transform;
	};
};



