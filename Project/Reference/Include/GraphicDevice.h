#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include <d3d9.h>

namespace Engine
{
	class DLL_DECL GraphicDevice : public SingletonInterface<GraphicDevice>
	{
	public:
		void Initialize(HWND Hwnd,const bool bFullScreen,
			const std::pair<uint32_t,uint32_t>ScreenSize) & noexcept; 
		inline IDirect3DDevice9* GetDevice()const &  { return _Device; }; 
	private:
		IDirect3D9* _SDK{ nullptr };  
		IDirect3DDevice9* _Device{ nullptr }; 
	};
};


