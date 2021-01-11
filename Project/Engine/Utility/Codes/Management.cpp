#include "Management.h"
#include "Timer.h"
#include "GraphicDevice.h"
#include <chrono>
#include "Controller.h"


void Engine::Management::Initialize(
	const HWND _Hwnd,
	const bool bFullScreen,
	const std::pair<uint32,uint32> ClientSize,
	const D3DMULTISAMPLE_TYPE MultiSample,
	const uint32 LimitFrame,
	const uint32 LimitDeltaMilliSec)&
{
	Hwnd = _Hwnd;
	this->ClientSize = ClientSize;

	Engine::GraphicDevice::Instance().Initialize(
		_Hwnd,
		bFullScreen,
		ClientSize,
		MultiSample);

	Engine::Timer::Instance().Initialize(
		LimitFrame, 
		std::chrono::milliseconds(LimitDeltaMilliSec),
		[this]() {BeforeUpdateEvent(); },
		[this](const float DeltaTime) {Update(DeltaTime); },
		[this]() {Render(); },
		[this]() {LastEvent(); });
}

void Engine::Management::GameLoop()&
{
	Timer::Instance().Update();
}

void Engine::Management::BeforeUpdateEvent()&
{
	Controller::Instance().Update();
}

void Engine::Management::Update(const float DeltaTime)&
{
	if(_CurrentScene)
		_CurrentScene->Update(DeltaTime);
}

void Engine::Management::Render()&
{
	// ·»´õ·¯ÀÇ ·»´õ¸µ ¼öÇà.
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

