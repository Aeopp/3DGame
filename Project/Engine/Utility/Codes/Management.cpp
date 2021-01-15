#include "Management.h"
#include "Timer.h"
#include "GraphicDevice.h"
#include <chrono>
#include "Controller.h"
#include "Sound.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "PrototypeManager.h"
#include "FontManager.h"
#include "FMath.hpp"

void Engine::Management::Initialize(
	const HWND _Hwnd,
	const HINSTANCE HInstance,
	const bool bFullScreen,
	const std::pair<uint32,uint32> ClientSize,
	const D3DMULTISAMPLE_TYPE MultiSample,
	const float DeltaMax,
	const std::filesystem::path& SoundPath)&
{
	Hwnd = _Hwnd;
	this->ClientSize = ClientSize;

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

	_Sound =Engine::Sound::Init(SoundPath);
	_Controller = Engine::Controller::Init(HInstance,_Hwnd);
	auto Device = _GraphicDevice->GetDevice(); 
	_Renderer = Engine::Renderer::Init(Device);
	_ShaderManager = Engine::ShaderManager::Init(Device);
	_PrototypeManager = Engine::PrototypeManager::Init();
	_FontManager = Engine::FontManager::Init();
}

Engine::Management::~Management() noexcept
{
	Sound::Reset();
	Controller::Reset();
	Timer::Reset();
	PrototypeManager::Reset();
	Renderer::Reset();
	ShaderManager::Reset();
	FontManager::Reset();

	GraphicDevice::Reset();
};

void Engine::Management::GameLoop()&
{
	_Timer->Update();
}

void Engine::Management::Event()&
{
	//system("cls");
	_Controller->Update();
	_Sound->Update();
}

void Engine::Management::Update(const float DeltaTime)&
{
	if(_CurrentScene)
		_CurrentScene->Update(DeltaTime);
}

void Engine::Management::Render()&
{
	_GraphicDevice->Begin();

	_Renderer->Render();

	_GraphicDevice->End();
}

void Engine::Management::LastEvent()&
{
	if (_CurrentScene)
		_CurrentScene->PendingKill();
}


