#include "MaterialInformation.h"
#include <istream>
#include <fstream>
#include <d3dx9.h>
#include "ResourceSystem.h"
#include <ostream>

void Engine::MaterialInformation::Load
(
	IDirect3DDevice9*const Device,  
	const std::filesystem::path& MatFilePath,
	const std::filesystem::path& MatFileName, 
	const std::wstring& TexFileExtenstion)&
{
	std::wifstream MatStream{ MatFilePath  / MatFileName };
	std::wstring TexLine{};
	std::wstring TexKey{};
	std::wstring TexFileName{};

	while (MatStream)
	{
		std::getline(MatStream, TexLine);
		if (TexLine.empty())continue;

		const size_t SeparatorOffset = TexLine.find('='); 
		if (SeparatorOffset == std::wstring::npos)break;

		TexKey.assign(std::begin(TexLine),std::begin(TexLine)+SeparatorOffset );
		TexFileName.assign(std::begin(TexLine) + (SeparatorOffset+1), std::end(TexLine));
		
		TexFileName += TexFileExtenstion;

		auto& ResourceSys = ResourceSystem::Instance;

		IDirect3DTexture9* _Texture{ nullptr };
		_Texture = ResourceSys->Get<IDirect3DTexture9>(TexFileName);

		if (_Texture == nullptr)
		{
			D3DXCreateTextureFromFile(Device, (MatFilePath /L"Texture" /TexFileName).c_str(), &_Texture);
			_Texture = ResourceSys->Insert<IDirect3DTexture9>(TexFileName, _Texture);
		}

		TextureMap.insert({ TexKey,_Texture});

		TexLine.clear();
		TexKey.clear();
		TexFileName.clear();
	}

	
	if (TextureMap.contains(L"Cavity"))
	{
		bCavity = true; 
	}

	PropPath = MatFilePath / (MatFileName.stem().wstring()+ L".props.txt"); 
	std::wifstream PropsStream{ PropPath };

	if (PropsStream.is_open())
	{
		 // ÆÄ½Ì ½ÃÀÛ ...... 

	}
}

IDirect3DTexture9* Engine::MaterialInformation::GetTexture(const std::wstring& TexKey) const&
{
	if (auto iter = TextureMap.find(TexKey);
		iter != std::end(TextureMap))
	{
		return iter->second;
	}
	else
	{
		return nullptr;  
	}
}

void Engine::MaterialInformation::PropSave()&
{
	std::wifstream  PropStream{ PropPath };

	if (PropStream.is_open())
	{

	}
}
