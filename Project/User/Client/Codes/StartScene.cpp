#include "..\\stdafx.h"
#include "StartScene.h"
#include "TombStone.h"
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

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);
	
	auto& FontMgr = RefFontManager();
	auto& Control = RefControl();
	auto& ResourceSys = RefResourceSys();
	auto& Manager = RefManager();
	auto& Proto =   RefProto();

	// 텍스쳐 리소스 추가. 
	{
		auto Tex = ResourceSys.Emplace<IDirect3DTexture9>(L"Texture_Logo", D3DXCreateTextureFromFile, Device,
			(App::ResourcePath / L"Texture" / L"Logo.jpg").c_str(), &LogoTexture);
	}

	// Shader...
	{
		Sample = Engine::ShaderFx::Load(Device, App::ResourcePath / L"Shader" / "Sample.hlsl", L"ShaderFx_Sample");
	}

	// 현재 씬 레이어 추가.
	{
		Manager.NewLayer<EnemyLayer>();
		Manager.NewLayer<StaticLayer>();
	}


	// 프로토타입 로딩.
	{
		
	// 	Proto.LoadPrototype<TombStone>(L"Static", Device ,Engine::RenderInterface::Group::NoAlpha);
		Proto.LoadPrototype<Player>(L"Static", Device,Engine::RenderInterface::Group::NoAlpha);
	}

	// 카메라 오브젝트 추가.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<StaticLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, &Control);
	}

	// 오브젝트 스폰
	{
		/*RefManager().NewObject<EnemyLayer,TombStone>(L"Static", L"TombStone_1" ,
			Vector3{ 1.f,1.f,1.f},Vector3{ 0,0,0 }, Vector3{ 0,0,0 });*/
		RefManager().NewObject<StaticLayer, Player>(L"Static", L"Player_0",
			Vector3{ 0.01f,0.01f,0.01f }, Vector3{ 0,0,0 }, Vector3{ 0,0,5 });

		RefManager().NewObject<StaticLayer, Player>(L"Static", L"Player_1",
			Vector3{ 0.01f,0.01f,0.01f }, Vector3{ 0,0,0 }, Vector3{ 10,0,5 });
	}

	LogoVtxBuf = ResourceSys.Get<IDirect3DVertexBuffer9>(L"VertexBuffer_Plane");
};

void StartScene::Event()&
{
	Super::Event();

	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("Help");
		{
			ImGui::Checkbox("Logo ?", &bLogo);
			if (ImGui::Button("Object KILL"))
			{
				for (auto& [TypeKey, TargetObjects] :RefManager().FindLayer<EnemyLayer>()->RefObjects())
				{
					for (auto& Target : TargetObjects)
					{
						auto TargetName = Target->GetName();

						if (TargetName.find(L"TombStone",0u) != std::wstring::npos)
						{
							Target->Kill();
						}
					}
				}
			}
		}
		ImGui::End();
	}
}
void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	
	// 오브젝트 레이어 검색.
	{
		auto _Target = RefManager().FindObject<EnemyLayer, TombStone>(L"TombStone_1");
		auto _TargetLayer = RefManager().FindLayer<EnemyLayer>();

		auto& LayerMap = RefManager().RefLayers();

		for (auto& _TombStone : RefManager().FindObjects<EnemyLayer, TombStone>())
		{
			_TombStone->GetName();
		}
		for (auto& _Camera: RefManager().FindObjects<StaticLayer, Engine::DynamicCamera>())
		{
			_Camera->GetName();
		}

		for (auto&  [_Key,_Objects]: RefManager().RefObjects<EnemyLayer>())
		{
			for (auto& _Object : _Objects)
			{
				_Object->GetName();
			};
		}
	}
};

void StartScene::Render()&
{
	if (bLogo)
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
		Fx->Begin(&iPassMax, 0);	// 1인자 : 현재 쉐이더 파일이 지닌 최대 pass개수, 2인자 : 시작하는 방식에  플래그
		Fx->BeginPass(0);
		Device->SetStreamSource(0, LogoVtxBuf, 0, sizeof(Vertex::LocationUV2D));
		Device->SetFVF(Vertex::LocationUV2D::FVF);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2u);
		Fx->EndPass();
		Fx->End();
	}
	else
	{

	}
}


