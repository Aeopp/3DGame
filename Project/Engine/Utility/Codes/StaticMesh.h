#pragma once
#include "Mesh.h"
#include <string>

namespace Engine
{
	class DLL_DECL StaticMesh : public Mesh
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
		inline ID3DXMesh* const GetMesh()const&;
	private:
		ID3DXMesh* _Mesh{ nullptr };
		ID3DXBuffer* Adjacency{ nullptr };
		ID3DXBuffer* SubSet{ nullptr };
		D3DXMATERIAL* Materials{ nullptr };
		uint32 SubSetCount{ 0u };
		std::vector<IDirect3DTexture9*> Textures{};
	};
}
inline ID3DXMesh* const Engine::StaticMesh::GetMesh()const&
{
	return _Mesh;
}