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
	// 텍스쳐 리소스 추가. 
	{

	}

	// 현재 씬 레이어 추가.
	{
		Manager.NewLayer<StaticLayer>();
	}


	// 프로토타입 로딩.
	{
		Proto.LoadPrototype<TombStone>(L"Static", Device, Engine::RenderInterface::Group::NoAlpha);

	}

	// 카메라 오브젝트 추가.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<StaticLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, &Control);
	}

	{
		auto TargetMap = RefManager().NewObject<StaticLayer, TombStone>(L"Static", L"TombStone_1",
			Vector3{ 100,100,100 }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 });

		// 네비게이션 메쉬를 설치할 지형을 월드 좌표계로 변환한 이후 피킹할 준비를 합니다.
		// TODO :: 때문에 네비메쉬를 저장할때에 네비메쉬의 정점 좌표들을
		// 지형 월드의 역변환을 수행해 지형의 로컬 좌표계와 맞추어 주는 작업을 하길 바람. 
		// 로딩 할시에는 항상 지형의 월드행렬을 받아서 곱해서 로딩.....
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

		ImGui::Begin("Navigation Mesh");
		{
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
			if (ImGui::Button("LinkNeighborCells"))
			{
				NaviMesh.CellNeighborLink();
			}
			NaviMesh.DebugLog();

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
		}
		ImGui::End();

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
							NaviMesh.InsertPoint(IntersectPt);
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


