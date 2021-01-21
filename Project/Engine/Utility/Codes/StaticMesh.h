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
		inline const auto& GetVertexLocations() const&; 
	private:
		ID3DXMesh* _Mesh{ nullptr };
		ID3DXBuffer* Adjacency{ nullptr };
		ID3DXBuffer* SubSet{ nullptr };
		D3DXMATERIAL* Materials{ nullptr };
		uint32 SubSetCount{ 0u };
		uint32 Stride{ 0u };
		uint32 VertexCount{ 0u };
		std::shared_ptr<std::vector<Vector3>> VertexLocations{};
		std::vector<IDirect3DTexture9*> Textures{};
	};
}
inline ID3DXMesh* const Engine::StaticMesh::GetMesh()const&
{
	return _Mesh;
}

inline const auto& Engine::StaticMesh::GetVertexLocations() const&
{
	return *VertexLocations;
}
