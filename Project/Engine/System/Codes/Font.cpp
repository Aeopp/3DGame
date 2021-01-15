#include "Font.h"

void Engine::Font::Initialize(
	IDirect3DDevice9* const Device, 
	const std::wstring& FontType, 
	const uint32 Width, 
	const uint32 Height, 
	const uint32 Weight)&
{
	this->Device =Device;

	D3DXFONT_DESC	FontDesc;
	ZeroMemory(&FontDesc, sizeof(D3DXFONT_DESC));

	FontDesc.CharSet = HANGUL_CHARSET;
	lstrcpy(FontDesc.FaceName, FontType.c_str());
	FontDesc.Width = Width;
	FontDesc.Height = Height;
	FontDesc.Weight = Weight;

	ID3DXFont* FontTemp;
	if (FAILED(D3DXCreateFontIndirect(Device, &FontDesc, &FontTemp)))
	{
		throw std::exception(__FUNCTION__);
	}

	_Font = DX::MakeUnique(FontTemp);

	ID3DXSprite* SpriteTemp;

	if (FAILED(D3DXCreateSprite(Device, &SpriteTemp)))
	{
		throw std::exception(__FUNCTION__);
	}

	Sprite =DX::MakeUnique(SpriteTemp);
}

void Engine::Font::Render(
	const std::wstring& String,
	const Vector2 Position,
	D3DXCOLOR Color)&
{
	RECT	Rect{ int32(Position.x), int32(Position.y) };
	Sprite->Begin(D3DXSPRITE_ALPHABLEND);
	_Font->DrawTextW(Sprite.get(),String.c_str(), 
		  static_cast<int32>(String.size()), &Rect, DT_NOCLIP, Color);
	Sprite->End();
}
