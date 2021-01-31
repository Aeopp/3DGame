#include "Management.h"
#include "Timer.h"
#include <chrono>
#include "Controller.h"
#include "Sound.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "PrototypeManager.h"
#include "FontManager.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui.h"
#include "Vertexs.hpp"
#include "CollisionSystem.h"

bool DebugMode{ false };

void ImGuiInitialize(const HWND Hwnd, IDirect3DDevice9* const Device);
void ImGuiFrameStart();

void Engine::Management::Initialize(
	const HWND _Hwnd,
	const HINSTANCE HInstance,
	const bool bFullScreen,
	const std::pair<uint32,uint32> ClientSize,
	const D3DMULTISAMPLE_TYPE MultiSample,
	const float DeltaMax,
	const std::filesystem::path& ResourcePath)&
{
	Hwnd = _Hwnd;
	this->ClientSize = ClientSize;
	this->ResourcePath = ResourcePath;

	_GraphicDevice = Engine::GraphicDevice::Init(
		_Hwnd,
		bFullScreen,
		ClientSize,
		MultiSample);

	_Timer = Engine::Timer::Init(
		DeltaMax,
		[this]() {Event(); },
		[this](const float DeltaTime) {Update(DeltaTime); },
		[this]() {Render(); },
		[this]() {LastEvent(); });

	_Sound =Engine::Sound::Init(ResourcePath/L"Sound");
	_Controller = Engine::Controller::Init(HInstance,_Hwnd);
	_CollisionSys = Engine::CollisionSystem::Init();
	auto Device = _GraphicDevice->GetDevice(); 
	_ShaderManager = Engine::ShaderManager::Init(Device);
	_PrototypeManager = Engine::PrototypeManager::Init();
	_FontManager = Engine::FontManager::Init();
	_ResourceSys = Engine::ResourceSystem::Init();
	_Renderer = Engine::Renderer::Init(Device);
	ImGuiInitialize(Hwnd, Device.get());

	CreateStaticResource();
	CreateCollisionDebugResource();
}

Engine::Management::~Management() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (_CurrentScene)
		_CurrentScene.reset();

	Sound::Reset();
	Controller::Reset();
	Timer::Reset();
	CollisionSystem::Reset();
	PrototypeManager::Reset();
	Renderer::Reset();
	ShaderManager::Reset();
	FontManager::Reset();
	ResourceSystem::Reset();
	GraphicDevice::Reset();
};



void Engine::Management::GameLoop()&
{
	_Timer->Update();
}

void Engine::Management::Event()&
{
	ImGuiFrameStart();

	_Controller->Update();
	_Sound->Update();
	_CurrentScene->Event();
}

void Engine::Management::Update(const float DeltaTime)&
{
	_CurrentScene->Update(DeltaTime);
	_CollisionSys->Update(DeltaTime);
}

void Engine::Management::Render()&
{
	Matrix InvView; 
	_GraphicDevice->GetDevice()->GetTransform(D3DTS_VIEW, &InvView);
	InvView =FMath::Inverse(InvView);
	const Vector3 LightLocation{ 0,0,0 };
	const Vector3 CameraLocation{ InvView ._41,InvView._42,InvView ._43};
	_ShaderManager->Update(CameraLocation,LightLocation);
	_GraphicDevice->Begin();
	
	_Renderer->Render();

	_FontManager->RenderFont(L"Font_Jinji", L"진지함", { 400,300 }, D3DXCOLOR{0.5f,1.f,0.5f,0.1f});
	_FontManager->RenderFont(L"Font_Default", L"기본", { 600,200}, D3DXCOLOR{ 0.5f,0.f,0.5f,1.f });

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	_GraphicDevice->End();
}

void Engine::Management::LastEvent()&
{
	_CurrentScene->PendingKill();

	if(SceneChangeEvent)
	{
		SceneChangeEvent();
		SceneChangeEvent = nullptr;
	}
}

void ImGuiInitialize(const HWND Hwnd, IDirect3DDevice9* const Device)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& ImGuiIoRef = ImGui::GetIO();
	ImGuiIoRef.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(Hwnd);
	ImGui_ImplDX9_Init(Device);
}

void ImGuiFrameStart()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
}

void Engine::Management::CreateStaticResource()&
{
		// 큐브용 인덱스 버퍼 로딩.
	auto Device = _GraphicDevice->GetDevice();

		IDirect3DIndexBuffer9* IdxBuffer{ nullptr };
		Device->CreateIndexBuffer(sizeof(Index::_16) * 12u,
			0,
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&IdxBuffer, nullptr);
		_ResourceSys->Insert<IDirect3DIndexBuffer9>
			(L"IndexBuffer_Cube", IdxBuffer);

		Index::_16* IndexBufferPtr{ nullptr };
		IdxBuffer->Lock(0, 0, (void**)&IndexBufferPtr, 0);

		// x+
		IndexBufferPtr[0]._1 = 1;
		IndexBufferPtr[0]._2 = 5;
		IndexBufferPtr[0]._3 = 6;

		IndexBufferPtr[1]._1 = 1;
		IndexBufferPtr[1]._2 = 6;
		IndexBufferPtr[1]._3 = 2;

		// x-
		IndexBufferPtr[2]._1 = 4;
		IndexBufferPtr[2]._2 = 0;
		IndexBufferPtr[2]._3 = 3;

		IndexBufferPtr[3]._1 = 4;
		IndexBufferPtr[3]._2 = 3;
		IndexBufferPtr[3]._3 = 7;

		// y+
		IndexBufferPtr[4]._1 = 4;
		IndexBufferPtr[4]._2 = 5;
		IndexBufferPtr[4]._3 = 1;

		IndexBufferPtr[5]._1 = 4;
		IndexBufferPtr[5]._2 = 1;
		IndexBufferPtr[5]._3 = 0;

		// y-
		IndexBufferPtr[6]._1 = 3;
		IndexBufferPtr[6]._2 = 2;
		IndexBufferPtr[6]._3 = 6;

		IndexBufferPtr[7]._1 = 3;
		IndexBufferPtr[7]._2 = 6;
		IndexBufferPtr[7]._3 = 7;

		// z+
		IndexBufferPtr[8]._1 = 7;
		IndexBufferPtr[8]._2 = 6;
		IndexBufferPtr[8]._3 = 5;

		IndexBufferPtr[9]._1 = 7;
		IndexBufferPtr[9]._2 = 5;
		IndexBufferPtr[9]._3 = 4;

		// z-
		IndexBufferPtr[10]._1 = 0;
		IndexBufferPtr[10]._2 = 1;
		IndexBufferPtr[10]._3 = 2;

		IndexBufferPtr[11]._1 = 0;
		IndexBufferPtr[11]._2 = 2;
		IndexBufferPtr[11]._3 = 3;

		IdxBuffer->Unlock();
	
}
void Engine::Management::CreateCollisionDebugResource()&
{
	auto Device = _GraphicDevice->GetDevice();

	IDirect3DTexture9* TextureNoCollision{ nullptr };
	IDirect3DTexture9* TextureCollision{ nullptr };

	{
		Device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &TextureNoCollision, NULL);
		D3DLOCKED_RECT		LockRect;
		ZeroMemory(&LockRect, sizeof(D3DLOCKED_RECT));
		TextureNoCollision->LockRect(0, &LockRect, NULL, 0);
		*((uint32*)LockRect.pBits) = D3DXCOLOR(1.f, 0.f, 0.f, 1.f);
		TextureNoCollision->UnlockRect(0);
		_ResourceSys->Insert<IDirect3DTexture9>(L"Texture_Collision", TextureNoCollision);
	}

	{
		Device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &TextureCollision, NULL);
		D3DLOCKED_RECT		LockRect;
		ZeroMemory(&LockRect, sizeof(D3DLOCKED_RECT));
		TextureCollision->LockRect(0, &LockRect, NULL, 0);
		*((uint32*)LockRect.pBits) = D3DXCOLOR(0.f, 1.f, 0.f, 1.f);
		TextureCollision->UnlockRect(0);
		_ResourceSys->Insert<IDirect3DTexture9>(L"Texture_NoCollision", TextureCollision);
	}
};


