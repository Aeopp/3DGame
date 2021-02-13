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
#include "Renderer.h"

void Tool::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);
	
	auto& FontMgr =     RefFontManager();
	auto& Control =     RefControl();
	auto& ResourceSys = RefResourceSys();
	auto& Manager =     RefManager();
	auto& Proto =       RefProto();
	auto& Renderer =    RefRenderer(); 

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

	}

	// 카메라 오브젝트 추가.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<StaticLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, &Control);
	}

	{
		Renderer.RefLandscape().Initialize(Device, MapWorld, App::ResourcePath /
			L"Mesh" / L"StaticMesh" / L"Landscape", L"Mountain.fbx");
		PickingPlanes = Renderer.RefLandscape().GetMapWorldCoordPlanes();
	}
};

void Tool::Event() & 
{
	Super::Event();

	if (ImGui::CollapsingHeader("Select"))
	{
		if (ImGui::Button("Navigation Mesh", ImVec2{ 70,40}) )
		{
			CurrentMode = Mode::NaviMesh;
		}ImGui::SameLine();
		if (ImGui::Button("Landscape", ImVec2{ 70,40}))
		{
			CurrentMode = Mode::Landscape;
		}
	}
	

	switch (CurrentMode)
	{
	case Tool::Mode::NaviMesh:
		NaviMeshTool();
		break;
	case Tool::Mode::Landscape:
		Landscape();
		break;
	default:
		break;
	}
	
}
void Tool::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}

void Tool::NaviMeshTool()&
{
	auto& NaviMesh = RefNaviMesh();

	ImGui::Begin("Navigation Mesh");
	{
		if (ImGui::Button("Save")) 
		{
			std::filesystem::path OpenPath = Engine::FileHelper::OpenDialogBox();
			NaviMesh.Save(OpenPath,MapWorld);
		}ImGui::SameLine(); 
		if (ImGui::Button("Load"))
		{
			std::filesystem::path OpenPath = Engine::FileHelper::OpenDialogBox();
			NaviMesh.Load(OpenPath,MapWorld);
		}ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			NaviMesh.Clear();
		}ImGui::Separator();
		if (ImGui::Button("Connecting Neighbors of Cells"))
		{
			NaviMesh.CellNeighborLink();
		}ImGui::Separator();
		if (NaviMeshCurrentSelectMarkeyKey != 0u)
		{
			static const ImVec2 Size{ 40,50 };
			static Vector3 PrevValue{ 0,0,0 };
			float x{ 0 }, y{ 0 }, z{ 0 };
			ImGui::BulletText("Location Control");

			{
				const char* Format = "None";
				ImGui::VSliderFloat("1",Size, &y, -0.01f, +0.01f,
					PrevValue.y == 0.0f ? "None" : PrevValue.y > 0.0f ? "Positive" : "Negative");
				PrevValue.y = y;
				const ImVec4 Color{ 0.f,1.f,0.f,1.f };
				ImGui::SameLine();
				ImGui::TextColoredV(Color, "Y", {});
			}
			{
				ImGui::SameLine();
				const char* Format = "None";
				ImGui::VSliderFloat("2", Size, &x, -0.01f, +0.01f,
					PrevValue.x == 0.0f ? "None" : PrevValue.x > 0.0f ? "Positive" : "Negative");
				PrevValue.x = x ;
				const ImVec4 Color{1.f,0.f,0.f,1.f  };
				ImGui::SameLine();
				ImGui::TextColoredV(Color, "X", {});
			}
			
			{
				ImGui::SameLine();
				const char* Format = "None";
				ImGui::VSliderFloat("3", Size, &z, -0.01f, +0.01f,
					PrevValue.z == 0.0f ? "None" : PrevValue.z > 0.0f ? "Positive" : "Negative");
				PrevValue.z = z;
				const ImVec4 Color{ 0.f,0.f,1.f,1.f };
				ImGui::SameLine();
				ImGui::TextColoredV(Color, "Z", {});
			}
			

			NaviMesh.MarkerMove(NaviMeshCurrentSelectMarkeyKey, Vector3{ x,y,z });
		}

		NaviMesh.DebugLog();
		ImGui::Text("Option ?");
		ImGui::RadioButton("Picking", &NavigationMeshModeSelect, 0); ImGui::SameLine(); 
		ImGui::RadioButton("Deletion", &NavigationMeshModeSelect, 1); ImGui::SameLine();
		ImGui::RadioButton("Observer", &NavigationMeshModeSelect, 2); 

		ImGui::BulletText("Debug Color");
		ImGui::ColorEdit4("Cell", NaviMesh.DefaultColor);
		ImGui::ColorEdit4("Select", NaviMesh.SelectColor);
		ImGui::ColorEdit4("Neighbor", NaviMesh.NeighborColor);
	}
	ImGui::End();

	POINT Pt;
	GetCursorPos(&Pt);
	ScreenToClient(App::Hwnd, &Pt);
	Vector3 Dir = { (float)Pt.x,(float)Pt.y,1.f };
	const Ray _Ray =
		FMath::GetRayScreenProjection
		(Dir, App::Device, App::ClientSize<float>.first, App::ClientSize<float>.second);

	auto& Control = RefControl();

	if (Control.IsDown(DIK_RIGHTCLICK))
	{
		if (NavigationMeshModeSelect == 0u)
		{
			NaviMeshCurrentSelectMarkeyKey = NaviMesh.InsertPointFromMarkers(_Ray);
			if (NaviMeshCurrentSelectMarkeyKey == 0u)
			{
				for (auto& CurTargetPlane : PickingPlanes)
				{
					float t = 0.0f;
					Vector3 IntersectPt;
					if (FMath::IsTriangleToRay(CurTargetPlane, _Ray, t, IntersectPt))
					{
						NaviMeshCurrentSelectMarkeyKey = NaviMesh.InsertPoint(IntersectPt);
					}
				}
			}
		}
		else if (NavigationMeshModeSelect == 1u)
		{
			NaviMesh.EraseCellFromRay(_Ray);
		}
		else if (NavigationMeshModeSelect == 2u)
		{
			NaviMeshCurrentSelectMarkeyKey=NaviMesh.SelectMarkerFromRay(_Ray);
			NaviMeshCurrentSelectCellKey = NaviMesh.SelectCellFromRay(_Ray);
		}
	}
};

void Tool::Landscape()&
{

};


