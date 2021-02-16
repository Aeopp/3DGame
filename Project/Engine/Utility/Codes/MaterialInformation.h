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
			const std::filesystem::path& MatFileName,
			const std::wstring& TexFileExtenstion /*= L".tga"*/)&;
		IDirect3DTexture9* GetTexture(const std::wstring& TexKey)const&;
		void PropSave()&;
	public:
		int32 bCavity = 0;
		int32 Contract = 4;
		float RimInnerWidth = 0.003f;
		float RimOuterWidth = 0.300f;
		float Power = 16.f;
		float SpecularIntencity = 0.5f;
		Vector4 RimAmtColor{ 1,1,1,1 };
		Vector4 AmbientColor{ 0.10f,0.10f,0.10f,1.f };
	private:
		std::filesystem::path PropPath{};
		std::unordered_map<std::wstring, IDirect3DTexture9*>TextureMap;
	};
	
}
