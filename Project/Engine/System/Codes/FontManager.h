#pragma once
#include "Font.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include <string>
#include <memory>
#include <unordered_map>
namespace Engine
{
 	class DLL_DECL FontManager  : public SingletonInterface<FontManager>
	{
	public:
		void Initialize()&;
	public:
		Font& AddFont(IDirect3DDevice9* const Device,
						const std::wstring& FontTag,
						const std::wstring& FontType,
						const uint32 Width,
						const uint32 Height,
						const uint32 Weight)&;
		void RenderFont(const std::wstring& Tag,
							const std::wstring& String,
							const Vector2 Position,
							const D3DXCOLOR Color)&;
	public:
		inline auto& RefFont(const std::wstring& Tag)&; 
	private:
		std::unordered_map<std::wstring, std::unique_ptr<Font>> FontMap;
	};
};



inline auto& Engine::FontManager::RefFont(const std::wstring& Tag)&
{
	return FontMap.find(Tag)->second;
}



