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

	static const Vector2 RenderTargetDebugRenderSize{ 90.f,90.f };

	Albedo3_Contract1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Albedo3_Contract1.DebugBufferInitialize({
		CurViewPort.Width / -2.f +(RenderTargetDebugRenderSize.x),
		CurViewPort.Height / +2.f - (RenderTargetDebugRenderSize.y * 1.f) },
		RenderTargetDebugRenderSize);

	Normal3_Power1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	Normal3_Power1.DebugBufferInitialize({
		CurViewPort.Width / -2.f +(RenderTargetDebugRenderSize.x),
		CurViewPort.Height / +2.f - (RenderTargetDebugRenderSize.y * 2.f) },
		RenderTargetDebugRenderSize);

	WorldLocation3_Depth1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A32B32G32R32F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	WorldLocation3_Depth1.DebugBufferInitialize({
		CurViewPort.Width / -2.f +(RenderTargetDebugRenderSize.x),
		CurViewPort.Height / +2.f - (RenderTargetDebugRenderSize.y * 3.f) },
		{ RenderTargetDebugRenderSize });

	CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.Initialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_A16B16G16R16F,
		D3DXCOLOR(0.f, 0.f, 0.f, 1.f));
	CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.DebugBufferInitialize({
		CurViewPort.Width / -2.f +(RenderTargetDebugRenderSize.x),
		CurViewPort.Height / +2.f - (RenderTargetDebugRenderSize.y * 4.f)},
		RenderTargetDebugRenderSize);

	ShadowDepth.Initialize(Device, CurViewPort.Width, CurViewPort.Height,
		D3DFMT_R32F,
		D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
	ShadowDepth.DepthStencilInitialize(Device, CurViewPort.Width, CurViewPort.Height, D3DFMT_D24X8);
	ShadowDepth.DebugBufferInitialize(
		{ CurViewPort.Width / -2.f +(RenderTargetDebugRenderSize.x*3.f),
	      CurViewPort.Height / +2.f - (RenderTargetDebugRenderSize.y * 1.f) },
		RenderTargetDebugRenderSize);

}
