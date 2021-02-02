#pragma once

#include "Component.h"
#include "RenderInterface.h"

namespace Engine
{
	class DLL_DECL Mesh : public Component 
	{
	public:
		using Super = Component;
	public:
		void Initialize(IDirect3DDevice9* const Device)&;
		virtual void Event(class Object* Owner) & override;
		virtual void Render()&;
	public:
		static const inline Property TypeProperty = Property::Render;
	protected:
		IDirect3DDevice9* Device{ nullptr };
	};
}