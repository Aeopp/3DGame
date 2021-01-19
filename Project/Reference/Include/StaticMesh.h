#pragma once
#include "Mesh.h"
#include <string>

namespace Engine
{
	class DLL_DECL StaticMesh  : public Mesh
	{
	public:
		using Super = Mesh;
	public:
		static const inline Property TypeProperty = Property::Render;
	public:
		void Initialize(IDirect3DDevice9* const Device,
			const RenderInterface::Group _Group ,
			const std::wstring& FilePath,
			const std::wstring& FileName)&;
		virtual void Event(class Object* Owner) & override;
	private:
		ID3DXMesh* _Mesh{ nullptr };
		ID3DXBuffer* Adjacency{ nullptr };
		ID3DXBuffer* SubSet{ nullptr };
		D3DXMATERIAL* _Mtrl{ nullptr };
		uint32 SubSetCount{ 0u };
		IDirect3DTexture9** Textures;
	};
}