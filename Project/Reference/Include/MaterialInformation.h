#pragma once
#include "DllHelper.h"
#include <unordered_map>
#include <string>
#include <d3d9.h>
#include <filesystem>
#include "ShaderFx.h"

namespace Engine
{
	class DLL_DECL MaterialInformation
	{
	public:
		struct DLL_DECL MaterialTexture
		{
		public:
			IDirect3DTexture9* Texture{ nullptr };
			std::string RegisterBindKey{};
		};
	public:
		void Load(IDirect3DDevice9* const Device, 
				const std::filesystem::path& MatFilePath,
			const std::filesystem::path& MatFileName,
			const std::wstring& TexFileExtenstion /*= L".tga"*/)&;
		void PropsLoad(const std::filesystem::path& PropsFilePath)&;
		IDirect3DTexture9* GetTexture(const std::string& MatTexKey)const&;
		IDirect3DTexture9* GetTextureFromRegisterKey(const std::string& RegisterTexKey)const&;
		void PropSave(std::filesystem::path PropsFilePath = {})&;
		void BindingTexture(ID3DXEffect*   _Fx)const&;
		void BindingMapping(const std::string& TexKey,
							const std::string& RegisterKey)&;
		static void SetUpDefaultTexture();
	public:
		std::string Name{"Material"};
		/// <summary>
		/// ������ ���� ������ ���̴� ����.
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
		float AlphaAddtive = 0.0f;

		Vector4 RimAmtColor{ 1,1,1,1 };
		Vector4 AmbientColor{ 0.10f,0.10f,0.10f,1.f };

		// �ʱ� �ε��� .mat ���Ͽ� �����ϴ� �ؽ��� ��
		std::unordered_map<std::string, MaterialTexture>MaterialTextureMap{};
		//// �����Ϳ��� ��Ÿ�ӿ� ���������� ���̴� �������Ϳ� �ؽ��ĸ� ���� . 
		/*std::unordered_map<std::string, IDirect3DTexture9*> BindingTextureMap{}; */
	private:
		static inline std::unordered_map<std::string, IDirect3DTexture9*> DefaultTextures{};
		std::filesystem::path PropPath{};
	};
	
}
