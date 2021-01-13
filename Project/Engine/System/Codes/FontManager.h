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
		inline auto& RefFont(const std::wstring& Tag)&; 
	private:
		std::unordered_map<std::wstring, std::unique_ptr<Font>> FontMap;
	};
};



inline auto& Engine::FontManager::RefFont(const std::wstring& Tag)&
{
	return FontMap.find(Tag)->second;
}



