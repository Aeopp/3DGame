#include "..\\stdafx.h"
#include "StartScene.h"
#include "ThirdPersonCamera.h"
#include "PlayerHead.h"
#include "NavigationMesh.h"
#include "Controller.h"
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
#include "PlayerWeapon.h"
#include "Renderer.h"
#include "ObjectEdit.h"
#include "Timer.h"
#include "Sound.h"

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
	bEditMode = false;
	Engine::Global::bDebugMode = false;

    Super::Initialize(Device);
	
	auto& FontMgr = RefFontManager();
	auto& Control = RefControl();
	auto& ResourceSys = RefResourceSys();
	auto& Manager = RefManager();
	auto& Proto =   RefProto();
	auto& Renderer = RefRenderer();
	auto& NaviMesh = RefNaviMesh();

	// 텍스쳐 리소스 추가. 
	{
		auto Tex = ResourceSys.Emplace<IDirect3DTexture9>(
			L"Texture_Logo", D3DXCreateTextureFromFile, Device,
			(App::ResourcePath / L"Texture" / L"Logo.jpg").c_str(), &LogoTexture);
	}

	// Shader...
	{
		Sample = Engine::ShaderFx::Load(Device, App::ResourcePath / L"Shader" / "Sample.hlsl", L"ShaderFx_Sample");
	}

	// 현재 씬 레이어 추가.
	{
		Manager.NewLayer<EnemyLayer>();
		Manager.NewLayer<Engine::NormalLayer>();
	}


	// 프로토타입 로딩.
	{

	}

	// 카메라 오브젝트 추가.
	{
		

		/*Manager.NewObject<Engine::NormalLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 4.f, 0.1f, 20000.f, Aspect, 333.f, &Control);*/
	}

	// 맵 정보 로딩
	{
		MapScale = { 0.1f , 0.1f, 0.1f };
		MapRotation = { 3.14f / 2.f,0.f,0.f };
		MapLocation = { 0,0,0 };

		auto& RefLandscape = Renderer.RefLandscape();
		RefLandscape.Initialize(Device, MapScale, MapRotation, MapLocation);

		std::vector<std::filesystem::path>DecoratorPaths
		{
			{ Engine::Global::ResourcePath / L"Mesh" / L"StaticMesh" / L"Decorator" / L""} ,
			{ Engine::Global::ResourcePath / L"Mesh" / L"StaticMesh" / L""},
			{ Engine::Global::ResourcePath / L"Mesh" / L"DynamicMesh" / L""}
		};

		for (const auto& CurPath : DecoratorPaths)
		{
			for (auto& TargetFileCurPath : std::filesystem::directory_iterator{ CurPath })
			{
				const auto& FileName = TargetFileCurPath.path().filename();

				if (FileName.has_extension())
				{
					RefLandscape.DecoratorLoad(CurPath, FileName);
				}
			}
		}

		RefLandscape.Load(App::ResourcePath / L"MapLoadInfo" / "SkyGarden.json");
		const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);
		NaviMesh.Load(App::ResourcePath / "NaviMesh" / "SkyGarden.json", MapWorld);
	}

	// 오브젝트 스폰
	{
		LogoVtxBuf = ResourceSys.Get<IDirect3DVertexBuffer9>(L"VertexBuffer_Plane");
		Renderer.SkyInitialize(App::ResourcePath / L"Mesh" / L"StaticMesh" / L"Sky" / L"SM_SkySphere.FBX");
	}
};

void StartScene::Event()&
{
	Super::Event();

	auto& _Control = RefControl();

	if (_Control.IsDown(DIK_F1))
	{
		Engine::Global::bDebugMode = !Engine::Global::bDebugMode;
	}

	if (_Control.IsDown(DIK_F2))
	{
		RefNaviMesh().bDebugRender = !RefNaviMesh().bDebugRender;
	}

	if (_Control.IsDown(DIK_F3))
	{
		RefTimer().TimerRegist(5.f, 0.0f, 5.1f, []()->bool {
				ObjectEdit::CaptureObjectLoad(App::ResourcePath / "SceneObjectCapture" / "BelatosWaveStart.json");
				return true;
				});
	}

	if (_Control.IsDown(DIK_TAB))
	{
		if (!bGameStart)
		{
			ObjectEdit::CaptureObjectLoad(App::ResourcePath / "SceneObjectCapture" /"SkyGarden.json");

			ObjectEdit::CaptureObjectLoad(App::ResourcePath / "SceneObjectCapture" / "Weapon.json");
		
			ObjectEdit::CaptureObjectLoad(App::ResourcePath / "SceneObjectCapture" /"NPC.json");

			

			RefSound().Play("BGM_TrainingRoom_01_B", 1.f, true, true);

			bGameStart = true;
		}
	}
}
void StartScene::Update(const float DeltaTime)&
{

	Super::Update(DeltaTime);
};

void StartScene::Render()&
{
	if (!bGameStart)
	{
		Matrix Projection;
		D3DXMatrixOrthoLH(&Projection, App::ClientSize<float>.first, App::ClientSize<float>.second,
			0.f, 1.f);
		ID3DXEffect* Fx = Sample->GetHandle();
		const Matrix Identity = FMath::WorldMatrix({ App::ClientSize<float>.first,App::ClientSize<float>.second,0.f }, { 0,0,0 }, { 0,0,0 });
		const Matrix View = FMath::Identity();
		Fx->SetMatrix("World", &Identity);
		Fx->SetMatrix("View", &View);
		Fx->SetMatrix("Projection", &Projection);
		Fx->SetTexture("Diffuse", LogoTexture);
		uint32 iPassMax = 0;
		Fx->Begin(&iPassMax, 0);	
		Fx->BeginPass(0);
		Device->SetStreamSource(0, LogoVtxBuf, 0, sizeof(Vertex::LocationUV2D));
		Device->SetFVF(Vertex::LocationUV2D::FVF);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2u);
		Fx->EndPass();
		Fx->End();
	}
}


