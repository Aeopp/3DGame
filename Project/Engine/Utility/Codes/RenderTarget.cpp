#include "imgui.h"
#include "RenderTarget.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"


static uint32 RenderTargetUniqueResourceID = 0u;

Engine::RenderTarget::RenderTarget()
	:UniqueResourceID{ RenderTargetUniqueResourceID++ }
{
}

void Engine::RenderTarget::Initialize(
	IDirect3DDevice9*const Device,
	const uint32 Width, const uint32 Height, 
	const D3DFORMAT Format, const D3DXCOLOR Color)&
{
	this->Device = Device;
	ClearColor = Color;
	static constexpr uint32 DefaultMipLevels = 1u;
	if (FAILED(D3DXCreateTexture(Device, Width, Height, DefaultMipLevels, D3DUSAGE_RENDERTARGET,
		Format, D3DPOOL_DEFAULT, &TargetTexture)))
	{
		throw std::exception(__FUNCTION__);
	}

	// 생성한 텍스쳐로부터 
	TargetTexture->GetSurfaceLevel(0u, &TargetSurface);

	const std::wstring RenderTargetUniqueResourceName =
		std::to_wstring(UniqueResourceID);

	if (TargetTexture)
	{
		ResourceSystem::Instance->Insert<IDirect3DTexture9>
			( L"RenderTargetTexture_"+RenderTargetUniqueResourceName , TargetTexture);
	}

	if (TargetSurface)
	{
		ResourceSystem::Instance->Insert<IDirect3DSurface9>
			(L"RenderTargetSurface_"+RenderTargetUniqueResourceName, TargetSurface);
	}
}

static constexpr uint8 MaxDeviceRenderTargetIndex = 3u;

void Engine::RenderTarget::BindGraphicDevice(const uint32 Index)&
{
	if (Index > MaxDeviceRenderTargetIndex)
	{
		MessageBox(Engine::Global::Hwnd,__FUNCTIONW__,__FUNCTIONW__,
			MB_OK);
	}
	//Device->GetRenderTarget(Index, &PrevTargetSurface);
	Device->SetRenderTarget(Index, TargetSurface);
}

//void Engine::RenderTarget::UnBindGraphicDevice(const uint32 Index)&
//{
//	if (Index > MaxDeviceRenderTargetIndex)
//	{
//		MessageBox(Engine::Global::Hwnd, __FUNCTIONW__, __FUNCTIONW__,
//			MB_OK);
//	}
//	//Device->SetRenderTarget(Index, PrevTargetSurface);
//	//PrevTargetSurface->Release();
//}

void Engine::RenderTarget::Clear()&
{
	// Device->GetRenderTarget(0, &PrevTargetSurface);
	Device->SetRenderTarget(0, TargetSurface);
	Device->Clear(0, NULL, D3DCLEAR_TARGET, ClearColor, 1.f, 0);
	//Device->SetRenderTarget(0, &PrevTargetSurface);
	//PrevTargetSurface->Release(); 
}

void Engine::RenderTarget::DebugBufferInitialize(
	const Vector2& ViewPortLeftTopAnchor,
	const Vector2& Size)&
{
	Stride = sizeof(Vertex::Screen);
	FVF = Vertex::Screen::FVF;

	Device->CreateVertexBuffer(
		Stride * 4u,
		D3DUSAGE_WRITEONLY, FVF,
		D3DPOOL_MANAGED,
		&VtxBuf, nullptr);

	Device->CreateIndexBuffer(sizeof(uint32) * 6u ,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX32,
		D3DPOOL_MANAGED,
		&IdxBuf, nullptr);

	auto& ResourceSys =ResourceSystem::Instance;
	const std::wstring UniqueResourceName = 
		std::to_wstring(UniqueResourceID);
	if (VtxBuf)
	{
		ResourceSys->Insert<IDirect3DVertexBuffer9>(
			L"RenderTarget_DebugVertexBuffer_" + UniqueResourceName,
			VtxBuf);
	}

	if (IdxBuf)
	{
		ResourceSys->Insert<IDirect3DIndexBuffer9>(
			L"RenderTarget_IndexVertexBuffer_" + UniqueResourceName,
			IdxBuf);
	}

	Vertex::Screen* VtxBufPtr{ nullptr };

	VtxBuf->Lock(0, 0, reinterpret_cast<void**>(&VtxBufPtr), 0);

	const float ViewPortLeft = ViewPortLeftTopAnchor.x;
	const float ViewPortTop = ViewPortLeftTopAnchor.y;

	VtxBufPtr[0].Homogeneous4D = 
				{ ViewPortLeft - 0.5f,ViewPortTop - 0.5f,0.f,1.f };
	VtxBufPtr[0].UV2D = {0.f,0.f };


	VtxBufPtr[1].Homogeneous4D = { ViewPortLeft + Size.x - 0.5f,
								  ViewPortTop - 0.5f,0.f,1.f };
	VtxBufPtr[1].UV2D = {1.f,0.f};


	VtxBufPtr[2].Homogeneous4D = { ViewPortLeft + Size.x - 0.5f,
									ViewPortTop  + Size.y - 0.5f,0.f,1.f };
	VtxBufPtr[2].UV2D = { 1.f,1.f };


	VtxBufPtr[3].Homogeneous4D = { ViewPortLeft- 0.5f,
							  ViewPortTop  +Size.y - 0.5f,0.f,1.f };
		VtxBufPtr[3].UV2D = {0.f,1.f};

	VtxBuf->Unlock();

	uint32* IdxBufPtr{ nullptr };
	IdxBuf->Lock(0u, 0u, reinterpret_cast<void**>(&IdxBufPtr), 0u);
	IdxBufPtr[0] = 0u;
	IdxBufPtr[1] = 1u;
	IdxBufPtr[2] = 2u;

	IdxBufPtr[3] = 0u;
	IdxBufPtr[4] = 2u;
	IdxBufPtr[5] = 3u;
	IdxBuf->Unlock();
}

void Engine::RenderTarget::BindShaderTexture(
	ID3DXEffect* const Fx, const std::string& ConstantTableName)&
{
	Fx->SetTexture(ConstantTableName.c_str(), TargetTexture);
}

void Engine::RenderTarget::RenderDebugBuffer()&
{
	Device->SetTexture(0, TargetTexture);
	Device->SetStreamSource(0, VtxBuf, 0u, Stride);
	Device->SetFVF(FVF);
	Device->SetIndices(IdxBuf);
	Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4u, 0u, 2u);
}





