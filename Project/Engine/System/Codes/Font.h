#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include <string>
#include "DxHelper.h"

namespace Engine
{
	class DLL_DECL Font 
	{
	public:
		void Initialize(IDirect3DDevice9* const Device,
						const std::wstring& FontType,
						const uint32 Width,
						const uint32 Height,
						const uint32 Weight)&;
		void Render(
			const std::wstring& String, 
			const Vector2 Position,
			D3DXCOLOR Color)&;
	private:
		IDirect3DDevice9*/*소유권 없음*/ Device{ nullptr };
		DX::UniquePtr<ID3DXFont> _Font{ nullptr };
		DX::UniquePtr<ID3DXSprite> Sprite{ nullptr };
	};
};



