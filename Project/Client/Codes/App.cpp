#include "stdafx.h"
#include "App.h"
#include "GraphicDevice.h"
#include "Timer.h"
#include "Management.h"
#include <chrono>
#include "Class.h"

void App::Initialize(const HWND _Hwnd)&
{
	this->_Hwnd = _Hwnd;

		Engine::Management::Instance().Initialize(
			_Hwnd,
			false,
			ClientSize,
			D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE,
			60u,
			30);
	//Engine::Management::Instance().Initialize(
	//_Hwnd,);

	//Engine::GraphicDevice::Instance().Initialize(
	//	_Hwnd,
	//	false,
	//	ClientSize,
	//	D3DMULTISAMPLE_16_SAMPLES);

	//using namespace std::chrono_literals;

	//Engine::Timer::Instance().Initialize(
	//	60u, 30ms,
	//	[](const float DeltaTime) {Engine::Management::Instance().Update(DeltaTime); },
	//	[]() {Engine::Management::Instance().PendingKill(); },
	//	[]() {Engine::Management::Instance().Render(); });
};


