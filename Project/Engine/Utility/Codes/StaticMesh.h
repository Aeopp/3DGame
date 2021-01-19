#pragma once
#include "Mesh.h"

namespace Engine
{
	class DLL_DECL StaticMesh  : public Mesh
	{
	public:
		using Super = Mesh;
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