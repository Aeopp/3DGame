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
	_CollisionSys->Update(DeltaTime;
}

void Engine::Management::Render()&
{
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

