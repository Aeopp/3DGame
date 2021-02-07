#include "..\\stdafx.h"
#include "Tool.h"
#include "FileHelper.h"
#include "StaticMesh.h"
#include <array>

#include "Controller.h"
#include "TombStone.h"
#include "Player.h"
#include "Shader.h"
#include "Vertexs.hpp"
#include "NavigationMesh.h"
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
#include <array>
#include "imgui.h"
#include "FontManager.h"
#include "UtilityGlobal.h"
#include "ShaderManager.h"
#include "Transform.h"
#include <commdlg.h>
#include <Windows.h>
#include <stdio.h>

void Tool::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);
	
	auto& FontMgr = RefFontManager();
	auto& Control = RefControl();
	auto& ResourceSys = RefResourceSys();
	auto& Manager = RefManager();
	auto& Proto = RefProto();
	
	_NaviMesh = &RefNaviMesh();
	// �ؽ��� ���ҽ� �߰�. 
	{

	}

	// ���� �� ���̾� �߰�.
	{
		Manager.NewLayer<StaticLayer>();
	}


	// ������Ÿ�� �ε�.
	{
		Proto.LoadPrototype<TombStone>(L"Static", Device, Engine::RenderInterface::Group::NoAlpha);

	}

	// ī�޶� ������Ʈ �߰�.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<StaticLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, &Control);
	}

	{
		auto TargetMap = RefManager().NewObject<StaticLayer, TombStone>(L"Static", L"TombStone_1",
			Vector3{ 100,100,100 }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 });

		// �׺���̼� �޽��� ��ġ�� ������ ���� ��ǥ��� ��ȯ�� ���� ��ŷ�� �غ� �մϴ�.
		// TODO :: ������ �׺�޽��� �����Ҷ��� �׺�޽��� ���� ��ǥ����
		// ���� ������ ����ȯ�� ������ ������ ���� ��ǥ��� ���߾� �ִ� �۾��� �ϱ� �ٶ�. 
		// �ε� �ҽÿ��� �׻� ������ ��������� �޾Ƽ� ���ؼ� �ε�.....
		auto TargetMesh = TargetMap->GetComponent<Engine::StaticMesh>();
		auto TargetTransform = TargetMap->GetComponent<Engine::Transform>();
		const Matrix TargetWorld = TargetTransform->UpdateWorld(); 

		std::vector<Vector3> Locations;		
		for (uint32 i = 0; i < TargetMesh->LocalVertexLocations->size(); ++i)
		{
			const Vector3 WorldVertexLocation = 
				FMath::Mul((*TargetMesh->LocalVertexLocations)[i], TargetWorld);

			Locations.push_back(WorldVertexLocation);
			if (Locations.size() == 3u)
			{
				PickingPlanes.push_back(PlaneInfo::Make({ Locations[0],
														  Locations[1],
														  Locations[2] }));
				Locations.clear();
			}
		}
	}
};

void Tool::Event() & 
{
	Super::Event();

	if (Engine::Global::bDebugMode)
	{
		auto& NaviMesh = RefNaviMesh();
		if (ImGui::Button("Save"))
		{
			std::filesystem::path OpenPath = Engine::FileHelper::OpenDialogBox();
			NaviMesh.Save(OpenPath);
		}
		if (ImGui::Button("Load"))
		{
			std::filesystem::path OpenPath = Engine::FileHelper::OpenDialogBox();
			NaviMesh.Load(OpenPath);
		}

		ImGui::SliderInt("ModeSlider", reinterpret_cast<int32*>(&NavigationMeshModeSelect),
			0, MaxNavigationMeshMode);

		switch (NavigationMeshModeSelect)
		{
		case 0u:
			ImGui::Text("Picking");
			break;
		case 1u:
			ImGui::Text("Eraser");
			break;
		case 2u:
			ImGui::Text("Preparing");
			break;
		default:
			break;
		}

		POINT Pt;
		GetCursorPos(&Pt);
		ScreenToClient(App::Hwnd, &Pt);
		Vector3 Dir = {(float)Pt.x,(float)Pt.y,1.f};
		const Ray _Ray =
			FMath::GetRayScreenProjection
			(Dir, App::Device, App::ClientSize<float>.first, App::ClientSize<float>.second);

		auto& Control  = RefControl();
		
		if (Control.IsDown(DIK_RIGHTCLICK))
		{
			if (NavigationMeshModeSelect == 0u)
			{
				if (false == NaviMesh.InsertPointFromMarkers(_Ray))
				{
					for (auto& CurTargetPlane : PickingPlanes)
					{
						float t = 0.0f;
						Vector3 IntersectPt;
						if (FMath::IsTriangleToRay(CurTargetPlane, _Ray, t, IntersectPt))
						{
							NaviMesh.InsertPoint(IntersectPt, true);
						}
					}
				}
			}
			if (NavigationMeshModeSelect == 1u)
			{
				NaviMesh.EraseCellFromRay(_Ray);
			}
		}
	}
}
void Tool::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};


