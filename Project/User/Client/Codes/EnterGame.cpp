#include "..\\stdafx.h"
#include "EnterGame.h"
#include "TombStone.h"
#include "Tool.h"
#include "StartScene.h"

#include "Player.h"
#include "Shader.h"
#include "Vertexs.hpp"
#include "Management.h"
#include "EnemyLayer.h"
#include "ExportUtility.hpp"
#include "ResourceSystem.h"
#include "FMath.hpp"
#include "App.h"
#include "StaticLayer.h"
#include <vector>
#include <array>
#include <numbers>
#include "Layer.h"
#include <iostream>
#include "DynamicCamera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "imgui.h"
#include <array>
#include "FontManager.h"
#include "UtilityGlobal.h"
#include "ShaderManager.h"

void EnterGame::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);
	
	auto& FontMgr = RefFontManager();
	auto& Control = RefControl();
	auto& ResourceSys = RefResourceSys();
	auto& Manager = RefManager();
	auto& Proto = RefProto();

	// 텍스쳐 리소스 추가. 
	{

	}

	// 현재 씬 레이어 추가.
	{
		Manager.NewLayer<EnemyLayer>();
		Manager.NewLayer<StaticLayer>();
	}


	// 프로토타입 로딩.
	{
		Proto.LoadPrototype<Engine::DynamicCamera>(L"Static",Device,App::Hwnd);
	}

	// 카메라 오브젝트 추가.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<StaticLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 100.f, Aspect, 10.f, &Control);
	}
};

void EnterGame::Event() & 
{
	Super::Event();
	auto& Manager = RefManager();

	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("Select Mode");
		if (ImGui::Button("GameStart"))
		{
			Manager.ChangeScene<StartScene>(); 
		}
		if (ImGui::Button("Tool"))
		{
			Manager.ChangeScene<Tool>();
		}
		ImGui::End(); 
	}
}
void EnterGame::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

};


