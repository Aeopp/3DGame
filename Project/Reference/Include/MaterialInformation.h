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
		void PropsLoad(const std::filesystem::path& PropsFilePath)&;
		IDirect3DTexture9* GetTexture(const std::wstring& TexKey)const&;
		void PropSave(std::filesystem::path PropsFilePath = {})&;
	public:
		std::string Name{"Material"};
		/// <summary>
		/// 툴에서 조절 가능한 쉐이더 변수.
		/// </summary>
		float DetailScale = 1.f; 
		float Contract = 4.0f;
		float RimInnerWidth = 0.003f;
		float RimOuterWidth = 0.300f;
		float Power = 16.f;
		float CavityCoefficient = 1.f;
		float DetailDiffuseIntensity = 1.f;
		float DetailNormalIntensity = 1.f;
		float SpecularIntencity = 0.5f;
		Vector4 RimAmtColor{ 1,1,1,1 };
		Vector4 AmbientColor{ 0.10f,0.10f,0.10f,1.f };

		std::unordered_map<std::wstring, IDirect3DTexture9*>TextureMap;
	private:
		std::filesystem::path PropPath{};
		
	};
	
}
