#include "..\\stdafx.h"
#include "EnterGame.h"
#include "PlayerHead.h"
#include "MapEdit.h"
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
#include "NormalLayer.h"
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
#include "ObjectEdit.h"
#include "PlayerWeapon.h"
#include "DelasaCenterChain.h"
#include "PlayerHair.h"
#include "ThirdPersonCamera.h"
#include "Belatos.h"



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
		Manager.NewLayer<Engine::NormalLayer>();
	}


	// 프로토타입 로딩.
	{
		Proto.LoadPrototype<Engine::DynamicCamera>(L"Static",Device,App::Hwnd);
		Proto.LoadPrototype<Engine::ThirdPersonCamera>(L"Static", Device, App::Hwnd);

		Proto.LoadPrototype<Player>(L"Static", Device);
		Proto.LoadPrototype<PlayerHead>(L"Static", Device);
		Proto.LoadPrototype<PlayerHair>(L"Static", Device);
		Proto.LoadPrototype<PlayerWeapon>(L"Static", Device); 
		Proto.LoadPrototype<DelasaCenterChain>(L"Static", Device);

		Proto.LoadPrototype<Belatos>(L"Static", Device);
	}

	// 카메라 오브젝트 추가.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<Engine::NormalLayer,Engine::DynamicCamera>(
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
		if (ImGui::Button("Editor"))
		{
			Manager.ChangeScene<MapEdit>();
		}
		if (ImGui::Button("ObjectEdit"))
		{
			Manager.ChangeScene<ObjectEdit>();
		}
		ImGui::End(); 
	}
}
void EnterGame::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

};


