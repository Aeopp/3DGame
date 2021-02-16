#pragma once
#include "DllHelper.h"
#include <unordered_map>
#include <string>
#include <d3d9.h>
#include <filesystem>

namespace Engine
{
	class DLL_DECL MaterialInformation
	{
	public:
		void Load(IDirect3DDevice9* const Device, 
				const std::filesystem::path& MatFilePath,
			const std::wstring& TexFileExtenstion /*= L".tga"*/)&;
	private:
		std::unordered_map<std::wstring, IDirect3DTexture9*>TextureMap;
	};
	
}
