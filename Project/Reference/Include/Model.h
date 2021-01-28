#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Vertexs.hpp"
#include <string>
#include <vector> 
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//struct Texture
//{
//	uint32 ID;
//	std::string Type;
//	std::string Path;
//};

namespace Engine
{
	class DLL_DECL Model
	{
	private:
		
	public:
		void LoadModel(const std::filesystem::path& Path,
						const std::filesystem::path& Name ,
			IDirect3DDevice9*const Device)&;	
		std::vector<IDirect3DVertexBuffer9*> Vertecies{};
		IDirect3DVertexBuffer9* VertexBuffer{};
		uint32 CountAllFacices = 0u;
		void Render(IDirect3DDevice9* Device)&;
		/*std::vector<Vertex::Texture > Vertices;
		std::vector<Index::_16> Indices;
		std::vector<IDirect3DTexture9*> Textures;
		void Render()
		{
			
		};*/
	};
}


