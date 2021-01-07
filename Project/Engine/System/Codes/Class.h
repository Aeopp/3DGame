#pragma once
#include <d3dx9.h>
#include <d3d9.h>

#define ENGINE_EXPORTS

#ifdef	ENGINE_EXPORTS
#define ENGINE_DLL		_declspec(dllexport)
#else
#define ENGINE_DLL		_declspec(dllimport)
#endif


class ENGINE_DLL CGraphicDev
{
public:
	LPDIRECT3DDEVICE9		GetDevice(void) { return m_pGraphicDev; }
public:
	HRESULT		Ready_GraphicDev(HWND hWnd,CGraphicDev** ppGraphicDev);

	void		Render_Begin(D3DXCOLOR Color);
	void		Render_End(void);

private:
	LPDIRECT3D9				m_pSDK = nullptr;
	LPDIRECT3DDEVICE9		m_pGraphicDev = nullptr;
	
};