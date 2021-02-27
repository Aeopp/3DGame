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

	Albedo3_Contract1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Albedo3_Contract1.DebugBufferInitialize({ 0.f,0.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize .second});

	Normal3_Power1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
			D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Normal3_Power1.DebugBufferInitialize({ 0.f,200.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize.second });

	WorldLocation3_Depth1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	WorldLocation3_Depth1.DebugBufferInitialize({ 0.f,400.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize.second });

	CavityRGB1_CavityAlpha1_NULL1_NULL1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	CavityRGB1_CavityAlpha1_NULL1_NULL1.DebugBufferInitialize({ 0.f,600.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize.second });

	RimRGB1_InnerWidth1_OuterWidth1_NULL1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, DefaultFormat,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	RimRGB1_InnerWidth1_OuterWidth1_NULL1.DebugBufferInitialize({ 0.f,800.f }, { RenderTargetDebugRenderSize.first,RenderTargetDebugRenderSize.second });
}
