#include "..\\stdafx.h"
#include "ObjectEdit.h"
#include "ExportUtility.hpp"
#include "ResourceSystem.h"
#include "FileHelper.h"
#include "PrototypeManager.h"
#include "NavigationMesh.h"
#include "App.h"
#include "FMath.hpp"
#include "DynamicCamera.h"
#include "Controller.h"
#include "NormalLayer.h"
#include "Renderer.h"
#include "Landscape.h"
#include "imgui.h"
#include "UtilityGlobal.h"


void ObjectEdit::Initialize(IDirect3DDevice9* const Device)&
{
	Super::Initialize(Device);

	auto& ResourceSys = RefResourceSys();
	auto& Proto = RefProto();
	auto& Manager = RefManager();
	auto& Control = RefControl();
	auto& Renderer = RefRenderer();

	// 현재 씬 레이어 추가.
	{
		Manager.NewLayer<Engine::NormalLayer>();
	}

	// 프로토타입 로딩.
	{

	}

	// 오브젝트 추가.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<Engine::NormalLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 10000.f, Aspect, 1000.f, &Control);
	}

	// 랜드스케이프. 
	{
		MapScale    = { 0.01f , 0.01f , 0.01f };
		MapRotation = { 3.14f / 2.f,0.f,0.f };
		MapLocation = { 0,0,0 };

		auto& RefLandscape = Renderer.RefLandscape();
		RefLandscape.Initialize(Device, MapScale, MapRotation, MapLocation, App::ResourcePath /
			L"Mesh" / L"StaticMesh" / L"Landscape", L"Mountain.fbx");

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
	}
}

void ObjectEdit::Event()&
{
	Super::Event();

	auto& Renderer = RefRenderer();
	auto& RefLandscape = Renderer.RefLandscape();
	auto& NaviMesh = RefNaviMesh();
	auto& Proto = RefProto();

	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);

	ImGui::Begin("Object Editor");
	if (ImGui::CollapsingHeader("File"))
	{
		if (ImGui::TreeNode("Landscape"))
		{
			if (ImGui::Button("DecoratorApply"))
			{
				DecoratorLoad(RefLandscape);
			}ImGui::SameLine();
			if (ImGui::Button("DecoratorClear"))
			{
				RefLandscape.Clear();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("NaviMesh"))
		{
			if (ImGui::Button("Apply"))
			{
				std::filesystem::path OpenPath = Engine::FileHelper::OpenDialogBox();
				NaviMesh.Load(OpenPath, MapWorld);
			}ImGui::SameLine();
			if (ImGui::Button("Clear"))
			{
				NaviMesh.Clear();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}

	}


	if (ImGui::TreeNode("SpawnInformation"))
	{
		if (ImGui::TreeNode("Transform"))
		{
			/*ImGui::SliderFloat3("Scale", (float*)(&CurSpawnParam.Scale),
				0.01f, 10.f);

			Vector3 SliderDegRotation = FMath::ToDegree(CurSpawnParam.Rotation);

			if (ImGui::SliderFloat3("Rotation", (float*)(&SliderDegRotation),
				-180, +180, "%f Deg"))
			{
				CurSpawnParam.Rotation = FMath::ToRadian(SliderDegRotation);
			}*/
			/*ImGui::SliderFloat3("Location", (float*)(&CurSpawnParam.Location),
				0.001f, 10000.f);*/

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	ImGui::End();

	auto CurSpawnEditEventList = Proto.Editor();

	auto& Control = RefControl();

	if (Control.IsDown(DIK_RIGHTCLICK))
	{
		POINT Pt;
		GetCursorPos(&Pt);
		ScreenToClient(App::Hwnd, &Pt);
		Vector3 Dir = { (float)Pt.x,(float)Pt.y,1.f };
		const Ray _Ray =
			FMath::GetRayScreenProjection
			(Dir, App::Device, App::ClientSize<float>.first, App::ClientSize<float>.second);

		if (auto IsIntersectPoint = RefLandscape.RayIntersectPoint(_Ray);
			IsIntersectPoint)
		{
			for (auto& CurSpawnEditEvent : CurSpawnEditEventList)
			{
				if (CurSpawnEditEvent)
				{
					CurSpawnParam.Location = *IsIntersectPoint;

					Engine::Object::SpawnReturnValue
						_SpawnObjReturnValue = CurSpawnEditEvent(CurSpawnParam);
				}
			}
		}
	}
};

void ObjectEdit::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};

void ObjectEdit::Render()&
{
	Super::Render();
}
void ObjectEdit::CaptureCurrentObjects()&
{
	auto& Manager = RefManager();
	for (const auto& [LayerKey, ObjectContainer] : Manager.RefLayers())
	{
		LayerKey;

		for (const auto& [ObjectTypeKey,ObjectContainer] : 
			ObjectContainer->RefObjects())
		{
			ObjectTypeKey;

			for (const auto& SaveObjectInstance : ObjectContainer)
			{

			}
		}
	}
};

void ObjectEdit::DecoratorLoad(Engine::Landscape& Landscape)&
{
	const auto& SelectPath = Engine::FileHelper::OpenDialogBox();
	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);
	Landscape.Load(SelectPath);
};
