#include "GraphicDevice.h"

void Engine::GraphicDevice::Initialize(
	HWND Hwnd, 
	const bool bFullScreen,
	const std::pair<uint32, uint32> ScreenSize)&
{
	_SDK = DX::MakeShared(Direct3DCreate9(D3D_SDK_VERSION));
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

	PresentParameter.MultiSampleType = D3DMULTISAMPLE_NONE;
	PresentParameter.MultiSampleQuality = 0u;

	//  ¾ÈÆ¼¾Ù¸®¾î½ÌÀ» ÃÖ°í Ä÷¸®Æ¼·Î ¼³Á¤.
	for (uint32 i = 1; i <= 16; ++i)
	{
		DWORD QualtyLevel{};
		if (SUCCEEDED(_SDK->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, D3DFMT_R8G8B8, FALSE, static_cast<_D3DMULTISAMPLE_TYPE>(i), &QualtyLevel)))
		{
			PresentParameter.MultiSampleType = static_cast<_D3DMULTISAMPLE_TYPE>(i);
			PresentParameter.MultiSampleQuality = QualtyLevel;
		}
	}


	PresentParameter.BackBufferWidth = ScreenSize.first;
	PresentParameter.BackBufferHeight = ScreenSize.second;
	PresentParameter.BackBufferFormat = D3DFMT_A8R8G8B8;
	PresentParameter.BackBufferCount = 1;

	
	PresentParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;
	PresentParameter.hDeviceWindow = Hwnd;

	PresentParameter.Windowed = !bFullScreen;
	PresentParameter.EnableAutoDepthStencil = TRUE;
	PresentParameter.AutoDepthStencilFormat = D3DFMT_D24S8;
	D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	PresentParameter.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	PresentParameter.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	PresentParameter.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	MultiSampleType = PresentParameter.MultiSampleType; 
	MultiSampleQuality = PresentParameter.MultiSampleQuality; 
	IDirect3DDevice9* _DevicePtr;
	if (FAILED(_SDK->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Hwnd, Flag, &PresentParameter, &_DevicePtr)))
		throw std::exception("CreateDevice");

	_Device = DX::MakeShared(_DevicePtr);
}

void Engine::GraphicDevice::Begin() & noexcept
{
	_Device->Clear(0, nullptr, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		0xff000000, 1.f, 0);

	_Device->BeginScene();

	for (uint8 i = 0; i < 8u; ++i)
	{
		_Device->SetSamplerState(i, D3DSAMPLERSTATETYPE::D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		_Device->SetSamplerState(i, D3DSAMPLERSTATETYPE::D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		_Device->SetSamplerState(i, D3DSAMPLERSTATETYPE::D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
		_Device->SetSamplerState(i, D3DSAMPLERSTATETYPE::D3DSAMP_MAXANISOTROPY, GetCaps().MaxAnisotropy);
	}
}

void Engine::GraphicDevice::End() & noexcept
{
	_Device->EndScene();
	_Device->Present(nullptr, nullptr, NULL, nullptr);
}
