#pragma once

#include "Component.h"
#include "RenderInterface.h"

namespace Engine
{
	class DLL_DECL Mesh : public Component , public RenderInterface
	{
	public:
		using Super = Component;
	public:
		void Initialize(IDirect3DDevice9* const Device,
						const RenderInterface::Group _Group)&;
		virtual void Event(class Object* Owner) & override;
	public:
		static const inline Property TypeProperty = Property::Standard;
	protected:
		IDirect3DDevice9* Device{ nullptr };
	};
}