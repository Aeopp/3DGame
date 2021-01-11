#pragma once
#include "SingletonInterface.h"
#include "DllHelper.h"
#include "DxHelper.h"
#include <d3d9.h>

namespace Engine
{
	class DLL_DECL GraphicDevice : public SingletonInterface<GraphicDevice>
	{
	public:
		void Initialize(HWND Hwnd, const bool bFullScreen,
			const std::pair<uint32_t, uint32_t>ScreenSize,
			const _D3DMULTISAMPLE_TYPE Anti_Aliasing)&;
		void Begin() & noexcept;
		void End() & noexcept;
		inline IDirect3DDevice9& GetDevice()const& { return *_Device; };
	private:
		DX::UniquePtr<IDirect3D9> _SDK{ nullptr };
		DX::UniquePtr<IDirect3DDevice9> _Device{ nullptr };
	};
};


