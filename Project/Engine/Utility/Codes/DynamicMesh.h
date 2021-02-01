#pragma once
#include "Mesh.h"
#include <string>

namespace Engine
{
	class DLL_DECL DynamicMesh : public Mesh
	{
	public:
		using Super = Mesh;
	public:
		static const inline Property TypeProperty = Property::Render;
	public:
		void Initialize(IDirect3DDevice9* const Device,
			const std::wstring& MeshResourceName)&;
		virtual void Event(class Object* Owner) & override;
		virtual void Render() & override;
	private:
		D3DXFRAME* RootFrame{ nullptr };
	};
}
