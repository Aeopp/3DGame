#include "GraphicDevice.h"

/// TODO:: 현재 안티엘리어싱 적용시 디바이스 생성이 되지 않는 이슈.
void Engine::GraphicDevice::Initialize(
	HWND Hwnd, 
	const bool bFullScreen,
	const std::pair<uint32, uint32> ScreenSize,
	const D3DMULTISAMPLE_TYPE Anti_Aliasing)&
{
	_SDK = DX::MakeUnique(Direct3DCreate9(D3D_SDK_VERSION));
	ZeroMemory(&DeviceCaps, sizeof(D3DCAPS9));

	if (FAILED(_SDK->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &DeviceCaps)))
		throw std::exception("GetDeviceCaps");

	uint32		Flag{ 0u };

	if (DeviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		Flag |= D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
	else
		Flag |= D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;

	D3DPRESENT_PARAMETERS PresentParameter;
	ZeroMemory(&PresentParameter, sizeof(D3DPRESENT_PARAMETERS));

	PresentParameter.BackBufferWidth = ScreenSize.first;
	PresentParameter.BackBufferHeight = ScreenSize.second;
	PresentParameter.BackBufferFormat = D3DFMT_A8R8G8B8;
	PresentParameter.BackBufferCount = 1;

	PresentParameter.MultiSampleType = Anti_Aliasing;
	PresentParameter.MultiSampleQuality = 0;
	PresentParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;
	PresentParameter.hDeviceWindow = Hwnd;

	PresentParameter.Windowed = !bFullScreen;
	PresentParameter.EnableAutoDepthStencil = TRUE;
	PresentParameter.AutoDepthStencilFormat = D3DFMT_D24S8;

	PresentParameter.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	PresentParameter.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	IDirect3DDevice9* _DevicePtr;
	if (FAILED(_SDK->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Hwnd, Flag, &PresentParameter, &_DevicePtr)))
		throw std::exception("CreateDevice");

	_Device = DX::MakeUnique(_DevicePtr);
}

void Engine::GraphicDevice::Begin() & noexcept
{
	_Device->Clear(0, nullptr, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		0xff0000ff, 1.f, 0);

	_Device->BeginScene();
}

void Engine::GraphicDevice::End() & noexcept
{
	_Device->EndScene();
	_Device->Present(nullptr, nullptr, NULL, nullptr);
}
