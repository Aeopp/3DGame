#include "FontManager.h"

void Engine::FontManager::Initialize()&
{

}

typename Engine::Font& 
	Engine::FontManager::AddFont(
		IDirect3DDevice9* const Device, 
		const std::wstring& FontTag, 
		const std::wstring& FontType, 
		const uint32 Width, 
		const uint32 Height, 
		const uint32 Weight)&
{
	auto& _Font = FontMap[FontTag] = std::make_unique<Font>();
	_Font->Initialize(Device, FontType, Width, Height, Weight);
	return *_Font;
}

void Engine::FontManager::RenderFont(const std::wstring& Tag, const std::wstring& String, const Vector2 Position, const D3DXCOLOR Color)&
{
	RefFont(Tag)->Render(String, Position, Color);
};

void Engine::FontManager::RenderRegist(
	const std::wstring& Tag, const std::wstring& String, const Vector2 Position, const D3DXCOLOR Color)&
{
	RenderFontCalls.push_back(
		[Tag, String, Position, Color, this]() {RenderFont(Tag, String, Position, Color); });
}

void Engine::FontManager::Render()&
{
	for (auto& RenderFontCall : RenderFontCalls)
	{
		if (RenderFontCall)
			RenderFontCall();
	}

	RenderFontCalls.clear();
}

