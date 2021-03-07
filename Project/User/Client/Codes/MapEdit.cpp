#include "..\\stdafx.h"
#include "MapEdit.h"
#include "FileHelper.h"
#include "StaticMesh.h"
#include <array>
#include "Controller.h"
#include "PlayerHead.h"
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
#include "NormalLayer.h"
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
#include "Landscape.h"
#include <ostream>
#include <fstream>
#include "StringHelper.h"
#include <array>



void MapEdit::Initialize(IDirect3DDevice9* const Device)&
{
     Super::Initialize(Device);
	
	auto& FontMgr =     RefFontManager();
	auto& Control =     RefControl();
	auto& ResourceSys = RefResourceSys();
	auto& Manager = RefManager();
	auto& Proto =       RefProto();
	auto& Renderer =    RefRenderer(); 

	_NaviMesh = &RefNaviMesh();
	// 텍스쳐 리소스 추가. 
	{

	}

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
			FMath::PI / 4.f, 0.1f, 20000.f, Aspect, 333.f, &Control);
	}

	{
		 MapScale     = { 1.f , 1.f, 1.f };
		 MapRotation = { 3.14f / 2.f,0.f,0.f };
		 MapLocation = { 0,0,0 };

		auto& RefLandscape = Renderer.RefLandscape();
		/*RefLandscape.Initialize(Device, MapScale, MapRotation,MapLocation, App::ResourcePath /
			L"Mesh" / L"StaticMesh" / L"Landscape", L"Mountain.fbx");*/
		RefLandscape.Initialize(Device, MapScale, MapRotation, MapLocation);
		
		std::vector<std::filesystem::path>DecoratorPaths
		{
			{ Engine::Global::ResourcePath / L"Mesh" / L"StaticMesh" / L"Decorator" / L""} ,
			{ Engine::Global::ResourcePath / L"Mesh" / L"StaticMesh" / L""},
			{ Engine::Global::ResourcePath / L"Mesh" / L"DynamicMesh" /  L""},
			{ Engine::Global::ResourcePath / L"Mesh" / L"StaticMesh" / L"Landscape" / L""}
		};

		for (const auto& CurPath : DecoratorPaths)
		{
			for (auto& TargetFileCurPath : std::filesystem::directory_iterator{ CurPath })
			{
				const auto& FileName = TargetFileCurPath.path().filename();
				
				if (FileName.has_extension())
				{
					RefLandscape.DecoratorLoad(CurPath, FileName);

					const auto PicturePath = (CurPath / L"Converted" / FileName.stem()).wstring() + L".png";
					static uint32 DecoID = 0u;
					DecoratorOption LoadDecoOpt;
					LoadDecoOpt.ID = DecoID++;
					LoadDecoOpt.Picture = ResourceSys.Get<IDirect3DTexture9>(PicturePath);

					if (LoadDecoOpt.Picture)
					{
						LoadDecoOpt.Width = *ResourceSys.GetAny<float>(PicturePath + L"Width");
						LoadDecoOpt.Height = *ResourceSys.GetAny<float>(PicturePath + L"Height");
					}
					else
					{
						D3DXCreateTextureFromFile(Device, PicturePath.c_str(), &LoadDecoOpt.Picture);
						if (LoadDecoOpt.Picture)
						{
							ResourceSys.Insert<IDirect3DTexture9>(PicturePath, LoadDecoOpt.Picture);
							D3DSURFACE_DESC ImageDesc;
							LoadDecoOpt.Picture->GetLevelDesc(0, &ImageDesc);
							LoadDecoOpt.Width = static_cast<float> (ImageDesc.Width);
							LoadDecoOpt.Height = static_cast<float> (ImageDesc.Height);
							ResourceSys.InsertAny<float>(PicturePath + L"Width", LoadDecoOpt.Width);
							ResourceSys.InsertAny<float>(PicturePath + L"Height", LoadDecoOpt.Height);
						};
					}

					
					DecoratorOpts.insert({ FileName  ,LoadDecoOpt });
				}
			}
		}

		RefLandscape.Load(App::ResourcePath / L"MapLoadInfo" / "SkyGarden.json");
	}

	D3DXCreateLine(Device, &LinearSpace);
	ResourceSys.Insert<ID3DXLine>(L"DebugLinearSpace", LinearSpace);

	Renderer.SkyInitialize(App::ResourcePath / L"Mesh" / L"StaticMesh" / L"SKy" / L"SM_SkySphere.FBX");
};

void MapEdit::Event() & 
{
	Super::Event();
	auto& Manager = RefManager();


	if (ImGui::CollapsingHeader("Select"))
	{
		if (ImGui::Button("Navigation Mesh", ImVec2{ 100,35}) )
		{
			CurrentMode = Mode::NaviMesh;
		}ImGui::SameLine();
		if (ImGui::Button("Landscape", ImVec2{ 70,35}))
		{
			CurrentMode = Mode::Landscape;
		}
	}

	switch (CurrentMode)
	{
	case MapEdit::Mode::NaviMesh:
		NaviMeshTool();
		break;
	case MapEdit::Mode::Landscape:
		Landscape();
		break;
	default:
		break;
	}
	
}
void MapEdit::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}

void MapEdit::Render()&
{
	Matrix View, Projection;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	Matrix ViewProjection = View * Projection;

	std::array<Vector3, 2u> XAxis{ Vector3{0,0,0},Vector3{ (std::numeric_limits<float>::max)() ,0,0} };
	std::array<Vector3, 2u> YAxis{ Vector3{0,0,0},Vector3{ 0.f,(std::numeric_limits<float>::max)() / 2.f,0} };
	std::array<Vector3, 2u> ZAxis{ Vector3{0,0,0},Vector3{ 0 ,0,(std::numeric_limits<float>::max)()} };
	LinearSpace->SetWidth(3.f);
	LinearSpace->Begin();

	LinearSpace->DrawTransform(XAxis.data(), XAxis.size(), &ViewProjection,
		D3DCOLOR_ARGB(100, 255, 0, 0));
	LinearSpace->DrawTransform(YAxis.data(), YAxis.size(), &ViewProjection,
		D3DCOLOR_ARGB(100, 0, 255, 0));
	LinearSpace->DrawTransform(ZAxis.data(), ZAxis.size(), &ViewProjection,
		D3DCOLOR_ARGB(100, 0, 0, 255));
	LinearSpace->End();
};


void MapEdit::NaviMeshTool()&
{
	auto& NaviMesh = RefNaviMesh();

	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);

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
	auto& Renderer = RefRenderer();
	auto& RefLandscape = Renderer.RefLandscape();

	if (Control.IsDown(DIK_RIGHTCLICK))
	{
		if (NavigationMeshModeSelect == 0u)
		{
			NaviMeshCurrentSelectMarkeyKey = NaviMesh.InsertPointFromMarkers(_Ray);
			if (NaviMeshCurrentSelectMarkeyKey == 0u)
			{
				std::map<float, Vector3> IntersectResults{}; 
				for (const auto& CurTargetPlane : RefLandscape.GetMapWorldCoordPlanes())
				{
					float t = 0.0f;
					Vector3 IntersectPt;
					if (FMath::IsTriangleToRay(CurTargetPlane, _Ray, t, IntersectPt))
					{
						IntersectResults[t] = IntersectPt; 
					}
				}
				if (false == IntersectResults.empty())
				{
					const Vector3 TheMostNearIntersectPt = IntersectResults.begin()->second; 
					NaviMeshCurrentSelectMarkeyKey = NaviMesh.InsertPoint(TheMostNearIntersectPt);
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


void MapEdit::Landscape()&
{
	auto& Renderer = RefRenderer();
	auto& RefLandscape = Renderer.RefLandscape();
	auto& RefMgr = RefManager();

	auto _Camera = RefMgr.FindObject<Engine::NormalLayer, Engine::DynamicCamera>(L"Camera");
	auto CameraTransform =_Camera->GetComponent<Engine::Transform>();
	const Vector3 CameraLocation =CameraTransform->GetLocation();
	const Vector3 CameraLook = CameraTransform->GetForward();

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
		
		bool PickSuccess = false; 
		if (SpawnTransformComboSelectItem == MapEdit::SpawnTransformItem::PickOvertheLandscape)
		{
			auto WeakPickDeco = RefLandscape.PushDecorator(SelectDecoKey, SpawnEditScale, SpawnEditRotation, SpawnEditLocation, bLandscapeInclude , _Ray);

			PickSuccess = !WeakPickDeco.first.expired(); 
			if (PickSuccess)
			{
				CurEditDecoInstance = WeakPickDeco;
			}
		}

		if (!PickSuccess)
		{
			CurEditDecoInstance = RefLandscape.PickDecoInstance(_Ray);
		}
	}
	
	uint32 DummyLableID = 0u;

	ImGui::Begin("Map Edit");

	{
		if (ImGui::CollapsingHeader("Decoratos"))
		{
			static char FilterFbxNameBuffer[MAX_PATH];
			ImGui::InputText("Filter Model Name", FilterFbxNameBuffer, MAX_PATH);
			for (auto& [DecoKey, DecoOpt] : DecoratorOpts)
			{
				std::string KeyA;
				KeyA.assign(std::begin(DecoKey), std::end(DecoKey));
				if (KeyA.find(FilterFbxNameBuffer)==std::string::npos && strlen(FilterFbxNameBuffer)!=0)
				{
					continue;
				}

				if ( ImGui::ImageButton(reinterpret_cast<void**>
					(DecoOpt.Picture), ImVec2{ 256,256 }))
				{
					switch (SpawnTransformComboSelectItem)
					{
					case MapEdit::SpawnTransformItem::CustomTransform:
						CurEditDecoInstance = RefLandscape.PushDecorator(DecoKey,
							SpawnEditScale, SpawnEditRotation, SpawnEditLocation ,
							bLandscapeInclude);
						break;
					case MapEdit::SpawnTransformItem::PickOvertheLandscape:
						SelectDecoKey = DecoKey;
						break;
					case MapEdit::SpawnTransformItem::InFrontOf:
						CurEditDecoInstance = RefLandscape.PushDecorator(DecoKey,
							SpawnEditScale, SpawnEditRotation,
							CameraLocation + CameraLook * InfrontOfScale,
							bLandscapeInclude
						);
						break;
					default:
						break;
					}
				}
				if (ImGui::CollapsingHeader( (KeyA+"_Material").c_str()))
				{
					if (auto DecoPtr = RefLandscape.GetDecorator(DecoKey);
						DecoPtr)
					{
						uint32 TextureID = 0u;
						for (auto& CurMesh : DecoPtr->Meshes)
						{
							if (ImGui::Button((CurMesh.MaterialInfo.Name +"_PropsSave").c_str()))
							{
								CurMesh.MaterialInfo.PropSave();
							}
							ImGui::BulletText("%s", CurMesh.MaterialInfo.Name.c_str());
							ImGui::SliderFloat((std::to_string(DummyLableID)
					    + "_Contract").c_str(), &CurMesh.MaterialInfo.Contract,1.f,20.f); 
							ImGui::ColorEdit4( (std::to_string(DummyLableID)+"_AmbientColor").c_str(), CurMesh.MaterialInfo.AmbientColor);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_Power").c_str(), &CurMesh.MaterialInfo.Power, 1.f, 100.f);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_CavityCoefficient").c_str(), &CurMesh.MaterialInfo.CavityCoefficient, 0.f, 2.2f);
							
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_SpecularIntencity").c_str(), &CurMesh.MaterialInfo.SpecularIntencity, 0.f, 1.f);
							ImGui::ColorEdit4((std::to_string(DummyLableID) + "_RimAmtColor").c_str(), CurMesh.MaterialInfo.RimAmtColor);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_RimOuterWidth").c_str(),&CurMesh.MaterialInfo.RimOuterWidth,0.f,1.f);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_RimInnerWidth").c_str(), &CurMesh.MaterialInfo.RimInnerWidth, 0.f, 1.f);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_DetailScale").c_str(), &CurMesh.MaterialInfo.DetailScale,1.f,100.f);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_DetailDiffuseIntensity").c_str(), &CurMesh.MaterialInfo.DetailDiffuseIntensity,0.f,2.f);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_DetailNormalIntensity").c_str(), &CurMesh.MaterialInfo.DetailNormalIntensity, 0.f,2.f);
							ImGui::SliderFloat((std::to_string(DummyLableID) + "_AlphaAddtive").c_str(),
								&CurMesh.MaterialInfo.AlphaAddtive, 0.0f, 1.0f);
							ImGui::Checkbox((std::to_string(DummyLableID) + "_bForwardRender").c_str(),
								&CurMesh.MaterialInfo.bForwardRender);

							DummyLableID++;
							const std::string   CurID = std::to_string(TextureID++);
							if (ImGui::CollapsingHeader( (KeyA+"_Texture_" + CurID).c_str()))
							{
								for (const auto& [TexNameKey, MtTex] : CurMesh.MaterialInfo.MaterialTextureMap)
								{
									ImGui::BulletText("%s = %s", (TexNameKey).c_str(),
										MtTex.RegisterBindKey.empty() ? "Not Binding" : MtTex.RegisterBindKey.c_str());

									const std::string TexEditPopupLabel = (KeyA+CurID + "_" + TexNameKey + "_Bind");
									if (ImGui::SmallButton(TexEditPopupLabel.c_str()))
									{
										ImGui::OpenPopup(TexEditPopupLabel.c_str());
									}

									ImGui::Image(reinterpret_cast<void**>(MtTex.Texture), { 128,128});

									if (ImGui::BeginPopup(TexEditPopupLabel.c_str()))
									{
										static std::array<char[256], 9u> InputBuffers
										{
											"",
											"DiffuseMap",
											"NormalMap",
											"CavityMap",
											"DetailDiffuseMap",
											"DetailNormalMap",
											"EmissiveMap",
										};
										for (uint32 i=0;i< InputBuffers.size();++i)
										{
											auto& CurBuf = InputBuffers[i];
											const std::string IndexStr = std::to_string(i);
											ImGui::InputText(IndexStr.c_str(), CurBuf, 256);
											ImGui::SameLine();
											if (ImGui::SmallButton(("Apply_" + IndexStr).c_str()))
											{
												CurMesh.MaterialInfo.BindingMapping(TexNameKey, CurBuf);
											}
										}
										ImGui::EndPopup();
									}
									ImGui::Separator();
								}
							}
							
							ImGui::Separator();
						}
					}
				}
		
				ImGui::BulletText("StaticMesh : File : %s", KeyA.c_str());
				ImGui::Separator();
			}
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Spawn Information Edit"))
		{
			ImGui::Combo("Select", 
				(int32*)&SpawnTransformComboSelectItem,
				SpawnTransformComboNames.data(), SpawnTransformComboNames.size());

			ImGui::Checkbox("bLandscapeInclude", &bLandscapeInclude);

			if (ImGui::CollapsingHeader("Scale"))
			{
				ImGui::SliderFloat3("Scale", (float*)&(SpawnEditScale), 0.01f, +100.f);
				ImGui::InputFloat3("_Scale", (float*)(SpawnEditScale));
			}
			if (ImGui::CollapsingHeader("Rotation"))
			{
				ImGui::SliderAngle("Yaw", &(SpawnEditRotation.y));
				ImGui::SliderAngle("Pitch", &(SpawnEditRotation.x));
				ImGui::SliderAngle("Roll", &(SpawnEditRotation.z));
				Vector3 RotationDeg = FMath::ToDegree(SpawnEditRotation);
				if (ImGui::InputFloat3("_Rotation", (float*)RotationDeg, "%.f Deg"))
				{
					SpawnEditRotation = FMath::ToRadian(RotationDeg);
				}
			}
			if (ImGui::CollapsingHeader("Location"))
			{
				ImGui::SliderFloat3("Location", (float*)&(SpawnEditLocation), -10000.f, +10000.f);
				ImGui::InputFloat3("_Location", (float*)SpawnEditLocation);
			}

			switch (SpawnTransformComboSelectItem)
			{
			case MapEdit::SpawnTransformItem::InFrontOf:
				ImGui::VSliderFloat("In front Of Scale", { 30,100 }, &InfrontOfScale, 1.f, 10000.f);
			default:
				break;
			}
		}
		ImGui::Separator();

		ImGui::Begin("SelectObject");
		if (auto CurEditDecoSharedInstance = CurEditDecoInstance.first.lock();
			CurEditDecoSharedInstance)
		{
			ImGui::BulletText(ToA(CurEditDecoInstance.second).c_str());
			ImGui::Checkbox("bLandscapeInclude", &CurEditDecoSharedInstance->bLandscapeInclude);
			if (ImGui::CollapsingHeader("Scale"))
			{
				static float Width = 10.f;
				ImGui::InputFloat("Width", &Width);
				ImGui::SliderFloat3("Scale", (float*)(&CurEditDecoSharedInstance->Scale), -Width, +Width);
				ImGui::InputFloat3("_Scale", (float*)&(CurEditDecoSharedInstance->Scale));
				float Allof = 0.f;
				ImGui::SliderFloat("AllOf",&Allof, -Width / 1000.f, +Width / 1000.f);
				if (false == FMath::AlmostEqual(Allof, 0.f))
				{
					CurEditDecoSharedInstance->Scale.x += Allof;
					CurEditDecoSharedInstance->Scale.y += Allof;
					CurEditDecoSharedInstance->Scale.z += Allof;
				}
			}
			ImGui::Separator();
			if (ImGui::CollapsingHeader("Rotation"))
			{
				ImGui::SliderAngle("Yaw", &(CurEditDecoSharedInstance->Rotation.y));
				ImGui::SliderAngle("Pitch", &(CurEditDecoSharedInstance->Rotation.x));
				ImGui::SliderAngle("Roll", &(CurEditDecoSharedInstance->Rotation.z));

				Vector3 Degree = CurEditDecoSharedInstance->Rotation;
				Degree.x = FMath::ToDegree(Degree.x);
				Degree.y = FMath::ToDegree(Degree.y);
				Degree.z = FMath::ToDegree(Degree.z);

				if (ImGui::InputFloat3("Rotation", (float*)&(Degree),
					"%.0f Deg"))
				{
					CurEditDecoSharedInstance->Rotation =
						{ FMath::ToRadian(Degree.x),
						FMath::ToRadian(Degree.y),
						FMath::ToRadian(Degree.z) };
				}
			}
			if (ImGui::CollapsingHeader("Location"))
			{
				Vector3 SliderLocation{ 0,0,0 };
				static float LocationResponsiveness = 1.f;
				ImGui::InputFloat("LocationResponsiveness", &LocationResponsiveness);
				ImGui::SliderFloat3("Location", (float*)&(SliderLocation), 
					-LocationResponsiveness, +LocationResponsiveness);
				ImGui::InputFloat3("_Location", (float*)&(CurEditDecoSharedInstance->Location));
				CurEditDecoSharedInstance->Location += SliderLocation;
			}
			ImGui::Separator();
			if (ImGui::Button("Copy"))
			{
				CurEditDecoInstance=
					RefLandscape.PushDecorator(CurEditDecoInstance.second,
					CurEditDecoSharedInstance->Scale, CurEditDecoSharedInstance->Rotation, CurEditDecoSharedInstance->Location,
					CurEditDecoSharedInstance->bLandscapeInclude);
			}ImGui::SameLine();
			if (ImGui::Button("Deselection"))
			{
				CurEditDecoInstance.first.reset();
			}
			if (ImGui::Button("Cleaning!") || RefControl().IsDown(DIK_DELETE))
			{
				if (CurEditDecoSharedInstance)
				{
					if (CurEditDecoSharedInstance->bLandscapeInclude)
					{
						CurEditDecoSharedInstance->bLandscapeInclude = false;
						RefLandscape.ReInitWorldPlanes();
					}
					CurEditDecoSharedInstance->bPendingKill = true;
				}
			}
		}
		ImGui::End();

		ImGui::Separator();
		if (ImGui::CollapsingHeader("File"))
		{
			if (ImGui::Button("DecoratorSave"))
			{
				DecoratorSave(RefLandscape);
			}ImGui::SameLine(); 
			if (ImGui::Button("DecoratorLoad"))
			{
				DecoratorLoad(RefLandscape); 
			} 
			if (ImGui::Button("DecoratorClear"))
			{
				RefLandscape.Clear();
			}ImGui::SameLine();
		}
		if (ImGui::Button("World Vertex Information Reconstruction From Decorator"))
		{
			RefLandscape.ReInitWorldPlanes();
		}
		ImGui::Checkbox("DebugSphereMesh ?", &RefLandscape.bDecoratorSphereMeshRender);

		const auto& DecoSaveInfoStr = RefLandscape.GetDecoratorSaveInfo();

		if (DecoSaveInfoStr.empty() == false)
		{
			ImGui::LogText("%s", DecoSaveInfoStr.c_str()); 
		};
	}

	ImGui::End();

	ImGui::Begin("RenderOption");
	ImGui::SliderFloat("FogDistance", &Renderer.FogDistance, 0.0f, 100000.f);
	ImGui::ColorEdit3("FogColor", (float*)&Renderer.FogColor );
	if (ImGui::TreeNode("DirectionalLight"))
	{
		ImGui::DragFloat4
		("Direction", (float*)&Renderer._DirectionalLight._LightInfo.Direction, 0.1f, -1.f, 1.f);
		D3DXVec4Normalize(&Renderer._DirectionalLight._LightInfo.Direction, &Renderer._DirectionalLight._LightInfo.Direction);
		ImGui::SliderFloat3("Location", (float*)&Renderer._DirectionalLight._LightInfo.Location, -10000.f, 10000.f);
		ImGui::SliderFloat("Shadow Distance", (float*)&Renderer._DirectionalLight._LightInfo.ShadowFar, 1000.f, 100000.f);
		ImGui::SliderFloat("Shadow OrthoProjectionFactor", (float*)&Renderer._DirectionalLight._LightInfo.ShadowOrthoProjectionFactor, 1.f, 5.f);

		
		ImGui::InputFloat("Shadow Depth Bias",
			&Renderer._DirectionalLight._LightInfo.ShadowDepthBias);
		ImGui::ColorEdit4("Color", (float*)&Renderer._DirectionalLight._LightInfo.LightColor);
		ImGui::TreePop();
	}
	
	ImGui::End();
}


void MapEdit::DecoratorSave(Engine::Landscape& Landscape)const&
{
	const auto& SelectPath = Engine::FileHelper::OpenDialogBox();
	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);
	Landscape.Save(SelectPath);
};

void MapEdit::DecoratorLoad(Engine::Landscape& Landscape)&
{
	const auto& SelectPath = Engine::FileHelper::OpenDialogBox();
	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);
	Landscape.Load(SelectPath);
};



