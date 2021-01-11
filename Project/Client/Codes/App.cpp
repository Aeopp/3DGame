#include "stdafx.h"
#include "App.h"
#include <chrono>
#include <filesystem>
#include "ExportUtility.h"
#include "Sound.h"

void App::Initialize(const HWND _Hwnd)&
{
	this->_Hwnd = _Hwnd;

	_Management = Engine::Management::Init(
			_Hwnd,
			false,
			ClientSize,
			D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE,
			60u,
			30,
		std::filesystem::current_path()/ ".." / ".."/"Resource" / "Sound");

	_Management->_Sound->Play("song", 1.f, true, true);
};


