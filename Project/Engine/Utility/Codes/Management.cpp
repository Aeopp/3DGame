#include "Management.h"
#include "Timer.h"
#include "GraphicDevice.h"
#include <chrono>
#include "Controller.h"
#include "Sound.h"



void Engine::Management::Initialize(
	const HWND _Hwnd,
	const bool bFullScreen,
	const std::pair<uint32,uint32> ClientSize,
	const D3DMULTISAMPLE_TYPE MultiSample,
	const uint32 LimitFrame,
	const uint32 LimitDeltaMilliSec,
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
		LimitFrame, 
		std::chrono::milliseconds(LimitDeltaMilliSec),
		[this]() {BeforeUpdateEvent(); },
		[this](const float DeltaTime) {Update(DeltaTime); },
		[this]() {Render(); },
		[this]() {LastEvent(); });

	_Sound =Engine::Sound::Init(SoundPath);
	_Controller = Engine::Controller::Init();
}

void Engine::Management::GameLoop()&
{
	_Timer->Update();
}

void Engine::Management::BeforeUpdateEvent()&
{
	_Controller->Update();
}

void Engine::Management::Update(const float DeltaTime)&
{
	if(_CurrentScene)
		_CurrentScene->Update(DeltaTime);

	_Sound->Update(DeltaTime);
}

void Engine::Management::Render()&
{
	// �������� ������ ����.
}

void Engine::Management::LastEvent()&
{
	if (_CurrentScene)
		_CurrentScene->PendingKill();
}

std::vector<std::shared_ptr<Engine::Layer>>& Engine::Management::RefLayers()&
{
	return _CurrentScene->RefLayers();
};

