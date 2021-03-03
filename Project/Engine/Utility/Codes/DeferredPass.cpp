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

	static const Vector2 RenderTargetDebugRenderSize{ 100.f,100.f};
	
	Albedo3_Contract1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Albedo3_Contract1.DebugBufferInitialize({ -860.f,+440.f}, RenderTargetDebugRenderSize);

	Normal3_Power1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
			D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Normal3_Power1.DebugBufferInitialize({ -860.f,+240.f}, RenderTargetDebugRenderSize);

	WorldLocation3_Depth1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	WorldLocation3_Depth1.DebugBufferInitialize({ -860.f,40.f}, { RenderTargetDebugRenderSize });

	CavityRGB1_CavityAlpha1_NULL_NULL1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	CavityRGB1_CavityAlpha1_NULL_NULL1.DebugBufferInitialize({ -860.f,-160.f}, RenderTargetDebugRenderSize);
	
	RimRGB1_InnerWidth1_OuterWidth1_NULL1.Initialize(Device, 
		CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	RimRGB1_InnerWidth1_OuterWidth1_NULL1.DebugBufferInitialize({ -860.f,-360.f },
		RenderTargetDebugRenderSize);

	uint32 ShadowDepthMapSize = 4096u;

	ShadowDepth.Initialize(Device, ShadowDepthMapSize,
		ShadowDepthMapSize,
		D3DFMT_R32F,
		D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
	ShadowDepth.DepthStencilInitialize(Device, ShadowDepthMapSize, ShadowDepthMapSize, D3DFMT_D24X8);
	ShadowDepth.DebugBufferInitialize({ -660.f,440.f }, RenderTargetDebugRenderSize);
}
