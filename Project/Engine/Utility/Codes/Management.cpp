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
	auto Device = _GraphicDevice->GetDevice(); 
	_ShaderManager = Engine::ShaderManager::Init(Device);
	_PrototypeManager = Engine::PrototypeManager::Init();
	_FontManager = Engine::FontManager::Init();
	_ResourceSys = Engine::ResourceSystem::Init();
	IDirect3DVertexBuffer9* TempVtxBuf{ nullptr };
	Device->CreateVertexBuffer(sizeof(Vertex::Default) * 8u, D3DUSAGE_WRITEONLY,
		Vertex::Default::FVF, D3DPOOL_MANAGED, &TempVtxBuf, nullptr);
	auto CubeVtxBuf = _ResourceSys->Create<IDirect3DVertexBuffer9>(L"Vertex_Cube", TempVtxBuf);
	Vertex::Default* VtxPtr;
	CubeVtxBuf->Lock(0, 0, (void**)&VtxPtr, 0);
	VtxPtr[0].Location.x = -1, VtxPtr[0].Location.y = -1, VtxPtr[0].Location.z = -1;
	VtxPtr[1].Location.x = -1.f, VtxPtr[1].Location.y = 1.f, VtxPtr[1].Location.z = -1;
	VtxPtr[2].Location.x = 1, VtxPtr[2].Location.y = 1, VtxPtr[2].Location.z = -1;
	VtxPtr[3].Location.x = 1, VtxPtr[3].Location.y = -1.f, VtxPtr[3].Location.z = -1;
	VtxPtr[4].Location.x = -1.f, VtxPtr[4].Location.y = -1.f, VtxPtr[4].Location.z = 1;
	VtxPtr[5].Location.x = -1, VtxPtr[5].Location.y = +1.f, VtxPtr[5].Location.z = 1;
	VtxPtr[6].Location.x = +1.f, VtxPtr[6].Location.y = +1.f, VtxPtr[6].Location.z = +1;
	VtxPtr[7].Location.x = 1.f, VtxPtr[7].Location.y = -1.f, VtxPtr[7].Location.z = +1;
	CubeVtxBuf->Unlock();
	IDirect3DIndexBuffer9* IdxBufTemp; 
	Device->CreateIndexBuffer(12 * sizeof(Index::_16),
		D3DUSAGE_WRITEONLY,
		Index::_16::Format,
		D3DPOOL_MANAGED, &IdxBufTemp, nullptr);

	auto CubeIdxBuf = _ResourceSys->Create<IDirect3DIndexBuffer9>(L"Index_Cube", IdxBufTemp);
	Index::_16* IdxPtr{ nullptr } ;
	CubeIdxBuf->Lock(0, 0, (void**)&IdxPtr, 0);

	IdxPtr[0 ]._1=0;   IdxPtr[0]._2 = 1; IdxPtr [0]._3 = 2;
	IdxPtr[1]._1 = 0; IdxPtr[1]._2 = 2; IdxPtr[1]._3 = 3;
	IdxPtr[2]._1 = 4; IdxPtr[2]._2 = 6; IdxPtr[2]._3 = 5;
	IdxPtr[3]._1 = 4; IdxPtr[3]._2 = 7; IdxPtr[3]._3 = 6;
	IdxPtr[4]._1 = 4; IdxPtr[4]._2 = 5; IdxPtr[4]._3 = 1;
	IdxPtr[5]._1 = 4; IdxPtr[5]._2 = 1; IdxPtr[5]._3 = 0;
	IdxPtr[6]._1 = 3; IdxPtr[6]._2 = 2; IdxPtr[6]._3 = 6;
	IdxPtr[7]._1 = 3; IdxPtr[7]._2 = 6; IdxPtr[7]._3 = 7;
	IdxPtr[8]._1 = 1; IdxPtr[8]._2 = 5; IdxPtr[8]._3 = 6;
	IdxPtr[9]._1 = 1; IdxPtr[9]._2 = 6; IdxPtr[9]._3 = 2;
	IdxPtr[10]._1 = 4; IdxPtr[10]._2 = 0; IdxPtr[10]._3 = 3;
	IdxPtr[11]._1 = 4; IdxPtr[11]._2 = 3; IdxPtr[11]._3 = 7;

	CubeIdxBuf->Unlock();
	

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

