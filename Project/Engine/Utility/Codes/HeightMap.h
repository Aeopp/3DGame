#pragma once
#include "RenderObject.h"

namespace Engine
{
	class HeightMap : public RenderObject
	{
	public:
		using Super = RenderObject;
	public:
		void Initialize(const RenderInterface::Group _Group)&;
		virtual void Update(const float DeltaTime) & override;
	};
};



