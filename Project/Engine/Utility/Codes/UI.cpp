#include "UI.h"
#include "ResourceSystem.h"
#include "Renderer.h"
#include "imgui.h"

void Engine::UI::Initialize(
	IDirect3DDevice9*const Device,
	const Vector2 Scale,
	const Vector2 Position,
	const std::filesystem::path& TexFullPath ,
	const float UIDepthZ)&
{
	static uint32 UniqueID = 0u;
	this->ID = UniqueID++;
	TexFileName = TexFullPath.filename().string();
	this->UIDepthZ = std::clamp(UIDepthZ, 0.0f, 1.f);
	this->Device = Device;
	Stride = sizeof(Vertex::UI);
	this->Scale = Scale;
	this->Position = Position;

	Device->CreateVertexBuffer(
		Stride * 4u,
		D3DUSAGE_WRITEONLY, NULL,
		D3DPOOL_MANAGED,
		&VtxBuf, nullptr);

	Device->CreateIndexBuffer(sizeof(uint32) * 6u,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX32,
		D3DPOOL_MANAGED,
		&IdxBuf, nullptr);

	static uint32 UniqueResourceID = 0u;

	auto& ResourceSys = ResourceSystem::Instance;
	const std::wstring UniqueResourceName =
		std::to_wstring(UniqueResourceID++);

	const std::wstring VertexName = ToW(typeid(Vertex::UI).name());
	VtxDecl = ResourceSys->Get<IDirect3DVertexDeclaration9>(VertexName);

	if (nullptr == VtxDecl)
	{
		VtxDecl = Vertex::UI::GetVertexDecl(Device);
		ResourceSys->Insert<IDirect3DVertexDeclaration9>(VertexName,
			VtxDecl);
	}


	if (VtxBuf)
	{
		ResourceSys->Insert<IDirect3DVertexBuffer9>(
			L"UI_VertexBuffer_" + UniqueResourceName,
			VtxBuf);
	}

	if (IdxBuf)
	{
		ResourceSys->Insert<IDirect3DIndexBuffer9>(
			L"UI_IndexBuffer_" + UniqueResourceName,
			IdxBuf);
	}

	Vertex::UI* VtxBufPtr{ nullptr };

	VtxBuf->Lock(0, 0, reinterpret_cast<void**>(&VtxBufPtr), 0);

	VtxBufPtr[0].NDC = { -1.f ,1.f,UIDepthZ };
	VtxBufPtr[0].TexCoord = { 0.f,0.f };

	VtxBufPtr[1].NDC = { 
		1.f, 
		1.f,UIDepthZ };
	VtxBufPtr[1].TexCoord = { 1.f,0.f };

	VtxBufPtr[2].NDC = { 
		1.f,
		-1.f,
		UIDepthZ };
	VtxBufPtr[2].TexCoord = { 1.f,1.f };

	VtxBufPtr[3].NDC = { 
		-1.f ,
		-1.f,
		UIDepthZ };
	VtxBufPtr[3].TexCoord = { 0.f,1.f };

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
	_Texture=ResourceSys->Get<IDirect3DTexture9>(TexFullPath);
	if (!_Texture)
	{
		D3DXCreateTextureFromFile(Device, TexFullPath.c_str(), &_Texture);
		if (_Texture)
		{
			ResourceSys->Insert< IDirect3DTexture9>(TexFullPath, _Texture);
		}
	}

	_ShaderFx.Initialize(L"DefaultUI");
}

void Engine::UI::Render(Engine::Renderer* const _Renderer)&
{
	if (bRender == false)return;

	auto Fx = _ShaderFx.GetHandle();
	Fx->SetTexture("ColorMap", _Texture);
	const Matrix Ortho = _Renderer->GetCurrentRenderInformation().OrthoProjection;
	Matrix UIMatrix;

	if (!WorldUI)
	{
		UIMatrix = FMath::WorldMatrix({ Scale.x,Scale.y,1.f }, { 0,0,0 }, { Position.x,Position.y,0 }) *
			Ortho;
	}
	else
	{
		UIMatrix = WorldUI.value() * _Renderer->GetCurrentRenderInformation().ViewProjection;
	}
	

	Fx->SetInt("Flag", Flag);
	Fx->SetFloat("CoolTimeHeight", CoolTimeHeight);
	Fx->SetFloat("AlphaFactor", AlphaFactor);
	Fx->SetMatrix("UIMatrix", &UIMatrix);
	Fx->SetFloatArray("AddColor", AddColor, 3u);
	
	Fx->CommitChanges();
	uint32 Pass = 0u;
	Fx->Begin(&Pass, NULL);
	for (uint32 i = 0; i < Pass; ++i)
	{
		Fx->BeginPass(i);
		{
			Device->SetStreamSource(0, VtxBuf, 0u, Stride);
			Device->SetVertexDeclaration(VtxDecl);
			Device->SetIndices(IdxBuf);
			Device->DrawIndexedPrimitive
			(D3DPT_TRIANGLELIST, 0, 0, 4u, 0u, 2u);
		}
		Fx->EndPass();
	}
	Fx->End();

	if (Engine::Global::bDebugMode)
	{
		if (ImGui::TreeNode( (TexFileName + std::to_string(ID)).c_str()))
		{
			ImGui::SliderFloat2("Position",Position,-1000.f, +1000.f);
			ImGui::SliderFloat("Scale", Scale, 0.0f, 1000.f);
			Scale.y = Scale.x;
			ImGui::TreePop();
		}
	}
}
