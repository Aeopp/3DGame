#pragma once
#include <Windows.h>
#include "SingletonInterface.h"
#include "DllHelper.h"
#include "DxHelper.h"
#include <d3d9.h>
#include "TypeAlias.h"

namespace Engine
{
	class DLL_DECL GraphicDevice : public SingletonInterface<GraphicDevice>
	{
	public:
		void Initialize(HWND Hwnd, const bool bFullScreen,
			const std::pair<uint32,uint32>ScreenSize,
			const D3DMULTISAMPLE_TYPE Anti_Aliasing)&;
		void Begin() & noexcept;
		void End() & noexcept;
		inline DX::SharedPtr<IDirect3DDevice9> GetDevice()& { return _Device; };
		inline D3DCAPS9 GetCaps()const& { return DeviceCaps; };
	private:
		D3DCAPS9 DeviceCaps;
		DX::SharedPtr<IDirect3D9> _SDK{ nullptr };
		DX::SharedPtr<IDirect3DDevice9> _Device{ nullptr };
	};
};


