#include "stdafx.h"
#include "App.h"
#include "Management.h"
#include <chrono>
#include <filesystem>

void App::Initialize(const HWND _Hwnd)&
{
	this->_Hwnd = _Hwnd;

	Engine::Management::Instance().Initialize(
			_Hwnd,
			false,
			ClientSize,
			D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE,
			60u,
			30,
		std::filesystem::current_path()/ ".." / "Resource" / "Sound");
};


