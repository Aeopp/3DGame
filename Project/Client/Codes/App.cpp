#include "stdafx.h"
#include "App.h"
#include "Management.h"
#include <chrono>

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
};


