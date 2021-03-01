#include "imgui.h"
#include "RenderTarget.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "StringHelper.h"


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
	this->Width = Width;
	this->Height = Height; 
	Device->CreateTexture(Width,Height, DefaultMipLevels, D3DUSAGE_RENDERTARGET,
		Format, D3DPOOL_DEFAULT, &TargetTexture, nullptr);

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

	_DebugBufferFx.Initialize(L"DebugBufferRenderFx");
}

void Engine::RenderTarget::DepthStencilInitialize(IDirect3DDevice9* Device, const uint32 Width, const uint32 Height, const D3DFORMAT Format)
{
	Device->CreateDepthStencilSurface(Width, Height,
		Format, D3DMULTISAMPLE_NONE, 0, TRUE ,  &TargetDepthStencil, nullptr);

	const std::wstring RenderTargetUniqueResourceName =
		std::to_wstring(UniqueResourceID);

	if (TargetDepthStencil)
	{
		ResourceSystem::Instance->Insert<IDirect3DSurface9>
			(L"RenderTargetDepthStencilSurface_" + RenderTargetUniqueResourceName, TargetDepthStencil);
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
	Device->SetRenderTarget(Index, TargetSurface);
}

void Engine::RenderTarget::BindDepthStencil()&
{
	Device->SetDepthStencilSurface(TargetDepthStencil);
}


void Engine::RenderTarget::Clear()&
{
	Device->SetRenderTarget(0, TargetSurface);
	Device->Clear(0, NULL, D3DCLEAR_TARGET, ClearColor, 1.f, 0);
}

void Engine::RenderTarget::ClearWithDepthStencil(const DWORD Flags )&
{
	Device->SetRenderTarget(0, TargetSurface);
	Device->SetDepthStencilSurface(TargetDepthStencil);

	Device->Clear(0, nullptr, Flags,
		ClearColor, 1.f, 0);
}

void Engine::RenderTarget::DebugBufferInitialize(
	const Vector2& ScreenPos, const Vector2& ScreenSize)&
{
	this->ScreenSize = ScreenSize;
	this->ScreenPos = ScreenPos;
	Stride = sizeof(Vertex::Screen);
	

	Device->CreateVertexBuffer(
		Stride * 4u,
		D3DUSAGE_WRITEONLY, NULL,
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

	const std::wstring VertexName = ToW(typeid(Vertex::Screen).name());


	DebugBufVtxDecl = ResourceSys->Get<IDirect3DVertexDeclaration9>(VertexName);

	if (nullptr == DebugBufVtxDecl)
	{
		DebugBufVtxDecl = Vertex::Screen::GetVertexDecl(Device);
		ResourceSys->Insert<IDirect3DVertexDeclaration9>(VertexName,
			DebugBufVtxDecl);
	}


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

	VtxBufPtr[0].NDC ={-1.f ,+1.f,0.f };
	VtxBufPtr[0].UV2D = { 0.f,0.f };

	VtxBufPtr[1].NDC = { +1.f, +1.f,0.f };
	VtxBufPtr[1].UV2D = { 1.f,0.f };

	VtxBufPtr[2].NDC = { +1.f,-1.f,0.f };
	VtxBufPtr[2].UV2D = { 1.f,1.f };

	VtxBufPtr[3].NDC = {-1.f,-1.f,0.f };
	VtxBufPtr[3].UV2D = { 0.f,1.f };

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
	auto Fx = _DebugBufferFx.GetHandle();
	Fx->SetTexture("BaseTexture", TargetTexture);
	Matrix Ortho;
	const Matrix ScreenMatrix=FMath::WorldMatrix({ ScreenSize.x,ScreenSize.y,0 }, { 0,0,0 }, { ScreenPos.x,ScreenPos.y,0 });
	D3DXMatrixOrthoLH(&Ortho, Engine::Global::ClientSize.first,
		Engine::Global::ClientSize.second, 0.0f, 1.f);
	Fx->SetMatrix("Ortho", &Ortho);
	Fx->SetMatrix("ScreenMatrix", &ScreenMatrix);

	Fx->CommitChanges();
	uint32 Pass = 0u;
	Fx->Begin(&Pass, NULL);
	for (uint32 i = 0; i < Pass; ++i)
	{
		Fx->BeginPass(i);
		{
			Device->SetStreamSource(0, VtxBuf, 0u, Stride);
			Device->SetVertexDeclaration(DebugBufVtxDecl);
			Device->SetIndices(IdxBuf);
			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4u, 0u, 2u);
		}
		Fx->EndPass();
	}
	Fx->End();
}





