#include "..\\stdafx.h"
#include "App.h"
#include "ExportUtility.hpp"
#include "Sound.h"
#include "FontManager.h"
#include "StartScene.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <chrono>
#include <filesystem>
#include "EnterGame.h"

void App::Initialize(const HWND _Hwnd, const HINSTANCE HInstance)&
{
	this->Hwnd = _Hwnd;

    Engine::Management::Init(
		_Hwnd,
		HInstance,
		false,
		ClientSize<uint32>,
		1.f/1.f,
		ResourcePath);

	Device = RefGraphic().GetDevice().get();

	StartSceneLoad();
}
void App::GameLoop()
{
	RefManager().GameLoop();
}

void App::StartSceneLoad()&
{
	Engine::Management::Instance->ChangeScene<EnterGame>();
};
