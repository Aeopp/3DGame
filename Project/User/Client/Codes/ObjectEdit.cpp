#include "..\\stdafx.h"
#include "ObjectEdit.h"
#include "StringHelper.h"
#include "ExportUtility.hpp"
#include "ResourceSystem.h"
#include "FileHelper.h"
#include "Transform.h"

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

#include <ostream>
#include <fstream>
#include <istream>

#include "FileHelper.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h> 
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include "StringHelper.h"
#include "EnemyLayer.h"



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
		Manager.NewLayer<EnemyLayer>();
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
			FMath::PI / 4.f, 0.1f, 1000.f, Aspect, 66.f, &Control);
	}

	// 랜드스케이프. 
	{
		MapScale    = { 0.1f , 0.1f, 0.1f };
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
	}
	
}

void ObjectEdit::Event()&
{
	Super::Event();

	auto& Renderer = RefRenderer();
	auto& RefLandscape = Renderer.RefLandscape();
	auto& NaviMesh = RefNaviMesh();
	auto& Proto = RefProto();

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
		Renderer.bDebugRenderTargetRender = !Renderer.bDebugRenderTargetRender;
	}


	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);

	ImGui::Begin("Object Editor");
	if (ImGui::CollapsingHeader("File"))
	{
		if (ImGui::TreeNode("Object Scene"))
		{
			if (ImGui::Button("Current Scene Object Instance Capture"))
			{
				CaptureCurrentObjects();
			}ImGui::SameLine();
			if (ImGui::Button("Capture Object Load"))
			{
				CaptureObjectLoad(Engine::FileHelper::OpenDialogBox());
			}
			ImGui::TreePop();
		}

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
};

void ObjectEdit::CaptureObjectLoad(const std::filesystem::path& FullPath)
{
	std::ifstream Is{ FullPath };
	using namespace rapidjson;
	if (!Is.is_open()) return;

	IStreamWrapper Isw(Is);
	Document _Document;
	_Document.ParseStream(Isw);

	if (_Document.HasParseError())
	{
		MessageBox(Engine::Global::Hwnd, L"Json Parse Error", L"Json Parse Error", MB_OK);
		return;
	}
	const Value& CaptureObjInstanceList = _Document["Object Instance List"];
	auto& Manager = RefManager();

	for (auto LayerIter = CaptureObjInstanceList.Begin();
		LayerIter != CaptureObjInstanceList.End(); ++LayerIter)
	{
		const std::string LayerKey = LayerIter->FindMember("LayerKey")->value.GetString();
		const auto& ObjectTypeArray = LayerIter->FindMember("ObjectTypeList")->value.GetArray();

		for (auto ObjectTypeIter = ObjectTypeArray.begin();
			ObjectTypeIter != ObjectTypeArray.end(); ++ObjectTypeIter)
		{
			const std::string ObjectTypeKey = 
				ObjectTypeIter->FindMember("ObjectTypeKey")->value.GetString();

			const auto& ObjectInstanceArray=
				ObjectTypeIter->FindMember("ObjectInstanceList")->value.GetArray();

			for (auto ObjectInstanceIter = ObjectInstanceArray.begin();
				ObjectInstanceIter !=ObjectInstanceArray.end(); ++ObjectInstanceIter)
			{
				const std::string SpawnObjName = ObjectInstanceIter->FindMember("Name")->value.GetString();

				const std::string PrototypeTag = ObjectInstanceIter->FindMember("PrototypeTag")->value.GetString();

				if (ObjectInstanceIter->FindMember("bCaptureable")->value.GetBool())
				{
					Engine::Object::SpawnParam _SpawnParam{};
					const auto& ScaleArr = ObjectInstanceIter->FindMember("Scale")->value.GetArray();
					_SpawnParam.Scale = { ScaleArr[0].GetFloat(),
						ScaleArr[1].GetFloat(),
						ScaleArr[2].GetFloat() };

					const auto& RotationArr = ObjectInstanceIter->FindMember("Rotation")->value.GetArray();
					_SpawnParam.Rotation = { RotationArr[0].GetFloat(),
											 RotationArr[1].GetFloat(),
											 RotationArr[2].GetFloat() };

					const auto& LocationArr = ObjectInstanceIter->FindMember("Location")->value.GetArray();
					_SpawnParam.Location = { LocationArr[0].GetFloat(),
											 LocationArr[1].GetFloat(),
											 LocationArr[2].GetFloat() };
					 
					Manager.NewObject(LayerKey, ObjectTypeKey, ToW(PrototypeTag), ToW(SpawnObjName), _SpawnParam);
				}
			}
		}
	};
}

void ObjectEdit::CaptureCurrentObjects()&
{
	const auto& SelectSavePath = Engine::FileHelper::OpenDialogBox();

	auto& Manager = RefManager();

	std::ofstream  PropStream{ SelectSavePath };

	if (PropStream.is_open())
	{
		using namespace rapidjson;
		StringBuffer StrBuf;
		PrettyWriter<StringBuffer> Writer(StrBuf);

		Writer.StartObject();

		Writer.Key("Object Instance List");
		Writer.StartArray();
		for (const auto& [LayerKey, ObjectContainer] : Manager.RefLayers())
		{
			Writer.StartObject();
			{
				Writer.Key("LayerKey");
				Writer.String(LayerKey.c_str());

				Writer.Key("ObjectTypeList");
				Writer.StartArray();
				{
					for (const auto& [ObjectTypeKey, ObjectContainer] :
						ObjectContainer->RefObjects())
					{
						Writer.StartObject();

						Writer.Key("ObjectTypeKey");
						Writer.String(ObjectTypeKey.c_str());

						Writer.Key("ObjectInstanceList");
						Writer.StartArray();
						for (const auto& SaveObjectInstance : ObjectContainer)
						{
							if (false == SaveObjectInstance->IsCapturable())continue;

							Writer.StartObject();
							{
								Writer.Key("Name");
								Writer.String(ToA(SaveObjectInstance->GetName()).c_str());

								Writer.Key("PrototypeTag");
								Writer.String(ToA(SaveObjectInstance->PrototypeTag).c_str());

								const Engine::Transform* InstanceTransform =
									SaveObjectInstance->GetComponent<Engine::Transform>();
								const bool bCaptureable = InstanceTransform != nullptr;

								Writer.Key("bCaptureable");
								Writer.Bool(bCaptureable);

								if (bCaptureable)
								{
									Engine::Object::SpawnParam InstanceSaveParam{};
									InstanceSaveParam.Scale = InstanceTransform->GetScale();
									InstanceSaveParam.Rotation = InstanceTransform->GetRotation();
									InstanceSaveParam.Location = InstanceTransform->GetLocation();

									Writer.Key("Scale");
									Writer.StartArray();
									Writer.Double(static_cast<double>(InstanceSaveParam.Scale.x));
									Writer.Double(static_cast<double>(InstanceSaveParam.Scale.y));
									Writer.Double(static_cast<double>(InstanceSaveParam.Scale.z));
									Writer.EndArray();

									Writer.Key("Rotation");
									Writer.StartArray();
									Writer.Double(static_cast<double>(InstanceSaveParam.Rotation.x));
									Writer.Double(static_cast<double>(InstanceSaveParam.Rotation.y));
									Writer.Double(static_cast<double>(InstanceSaveParam.Rotation.z));
									Writer.EndArray();

									Writer.Key("Location");
									Writer.StartArray();
									Writer.Double(static_cast<double>(InstanceSaveParam.Location.x));
									Writer.Double(static_cast<double>(InstanceSaveParam.Location.y));
									Writer.Double(static_cast<double>(InstanceSaveParam.Location.z));
									Writer.EndArray();
								}
							}
							Writer.EndObject();
						}
						Writer.EndArray();

					Writer.EndObject();
					}
				}
				Writer.EndArray();
			}
			Writer.EndObject();
		}
		Writer.EndArray();

		Writer.EndObject();

		PropStream << StrBuf.GetString();
	}
};

void ObjectEdit::DecoratorLoad(Engine::Landscape& Landscape)&
{
	const auto& SelectPath = Engine::FileHelper::OpenDialogBox();
	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);
	Landscape.Load(SelectPath);
};
