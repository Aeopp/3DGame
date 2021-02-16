#include "MaterialInformation.h"
#include <istream>
#include <fstream>
#include <d3dx9.h>
#include "ResourceSystem.h"

void Engine::MaterialInformation::Load
(
	IDirect3DDevice9*const Device,  
	const std::filesystem::path& MatFilePath,
	const std::wstring& TexFileExtenstion)&
{
	std::wifstream MatStream{ MatFilePath };
	std::wstring   Separator, MatKey, TexFileName{};

	while (MatStream)
	{
		MatStream >> MatKey;
		MatStream >> Separator;
		MatStream >> TexFileName;

		TexFileName += TexFileExtenstion;

		auto& ResourceSys = ResourceSystem::Instance;


		IDirect3DTexture9* _Texture{ nullptr };
		_Texture = ResourceSys->Get<IDirect3DTexture9>(TexFileName);

		if (_Texture == nullptr)
		{
			D3DXCreateTextureFromFile(Device, (MatFilePath / TexFileName).c_str(), &_Texture);
			_Texture = ResourceSys->Insert<IDirect3DTexture9>(TexFileName, _Texture);
		}

		TextureMap.insert({TexFileName,_Texture});

		Separator.clear();
		MatKey.clear();
		TexFileName.clear();
	}
}
