#include "DeferredPass.h"
#include "imgui.h"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"

void Engine::DeferredPass::Initialize(IDirect3DDevice9* const Device)&
{
	this->Device = Device;
	DefaultInitialize();
}

void Engine::DeferredPass::DefaultInitialize()&
{
	D3DVIEWPORT9 CurViewPort{};
	Device->GetViewport(&CurViewPort);
	static constexpr D3DFORMAT DefaultFormat = D3DFMT_A32B32G32R32F;

	static constexpr std::pair<uint32,uint32> RenderTargetDebugRenderSize{ 200,200 };

	Albedo.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Albedo.DebugBufferInitialize({ 0.f,0.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize .second});

	Normal.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
			D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Normal.DebugBufferInitialize({ 0.f,200.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize.second });

	WorldLocations.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	WorldLocations.DebugBufferInitialize({ 0.f,400.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize.second });
}
