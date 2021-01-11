#pragma once
#include "Object.h"
#include "RenderInterface.h"

namespace Engine
{
	class DLL_DECL HeightMap : public Object , public RenderInterface
	{
	public:
		using Super = Object; 
	public:
		void Initialize()&;
	public:
		virtual void Update(const float DeltaTime)& abstract;
		virtual void Render()& abstract;
	};
};



