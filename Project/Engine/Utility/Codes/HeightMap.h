#pragma once
#include "RenderObject.h"

namespace Engine
{
	class  DLL_DECL HeightMap : public RenderObject
	{
	public:
		using Super = RenderObject;
	public:
		void Initialize(const RenderInterface::Group _Group)&;
		virtual void Update(const float DeltaTime) & override;
		virtual void Render() & override;
	};
};



