#include "..\\stdafx.h"
#include "StartScene.h"
#include "TombStone.h"
#include "Vertexs.hpp"
#include "Management.h"
#include "HeightMap.h"
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
#include "FontManager.h"




struct Location3DUV
{
    Vector3 Location;
    Vector3 UV;
    static inline constexpr uint32_t FVF = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);
};

struct _16_t
{
    WORD _1, _2, _3;
    static inline constexpr D3DFORMAT Format = D3DFMT_INDEX16;
};

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);

	RefFontManager().AddFont(Device, L"Font_Default", L"πŸ≈¡", 15, 20, FW_HEAVY);
	RefFontManager().AddFont(Device, L"Font_Jinji", L"±√º≠", 15, 20, FW_THIN);

	auto* _Control = &RefControl();

	   D3DLOCKED_RECT LockRect; 
	   IDirect3DTexture9* ResourcePtr{ nullptr };

	   RefResourceSys().Insert<IDirect3DVertexDeclaration9>(
		   L"QQ", Vertex::Texture::GetVertexDecl(Device) );

	  auto Tex = RefResourceSys().Emplace<IDirect3DTexture9>(
		L"Texture",D3DXCreateTextureFromFile,Device,
		  (App::ResourcePath/ L"Texture" / L"Player0.jpg").c_str(),&ResourcePtr);

	Tex->LockRect(0, &LockRect, 0, D3DLOCK_DISCARD);
	Tex->UnlockRect(0);

	RefManager().NewLayer<EnemyLayer>();
	RefManager().NewLayer<StaticLayer>();

	constexpr float Aspect = App::ClientSize<float>.first /							   App::ClientSize<float>.second;

	static bool bInit = false;

	if (bInit == false)
	{
		RefProto().LoadPrototype<Engine::HeightMap>(L"Static",
			Device, Engine::RenderInterface::Group::Enviroment);
		RefProto().LoadPrototype<Engine::DynamicCamera>(L"Static", Device, App::Hwnd);
		RefProto().LoadPrototype<TombStone>(L"Static", Device);

		for (size_t i = 0; i < 16; ++i)
		{
			RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap" + std::to_wstring(i));
		}
		bInit = true;
	}
	else
	{
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap");
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap2");
	}


	RefManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 1.f, _Control);

	{
		ID3DXBuffer* Adjacency{ nullptr };
		ID3DXBuffer* SubSet{ nullptr };
		DWORD SubSetCount{ 0u };
		ID3DXMesh* _Mesh{ nullptr };
		D3DXMATERIAL* Materials;
		std::filesystem::path Path = App::ResourcePath / L"Mesh" / L"StaticMesh" / L"TombStone";
		std::filesystem::path Name = L"TombStone.x";

		RefResourceSys().Emplace<ID3DXMesh>(L"StaticMesh_Mesh_TombStone"
			, D3DXLoadMeshFromX, (Path / Name).c_str(),
			D3DXMESH_MANAGED,
			Device, &Adjacency, &SubSet, nullptr,
			&SubSetCount, &_Mesh);

		RefResourceSys().Insert<ID3DXBuffer>(L"StaticMesh_Adjacency_TombStone",
			Adjacency);

		RefResourceSys().Insert< ID3DXBuffer>(L"StaticMesh_SubSet_TombStone",
			SubSet);

		RefResourceSys().InsertAny(L"StaticMesh_SubSetCount_TombStone", SubSetCount);

		Materials = static_cast<D3DXMATERIAL*>(SubSet->GetBufferPointer());
		std::vector<IDirect3DTexture9*>Textures;
		Textures.resize(SubSetCount);

		for (uint32 Idx = 0u; Idx < SubSetCount; ++Idx)
		{
			const std::string TextureFileName = Materials[Idx].pTextureFilename; 
			std::wstring TextureFileNameW;
			TextureFileNameW.assign(std::begin(TextureFileName), std::end(TextureFileName));

			IDirect3DTexture9* _TexturePtr{ nullptr }; 

			Textures[Idx] = 
			RefResourceSys().Emplace<IDirect3DTexture9>
				(L"StaticMesh_Textures_TombStone_"+TextureFileNameW, D3DXCreateTextureFromFile,
				Device, (Path / TextureFileNameW).c_str(), &_TexturePtr);
		}
		RefResourceSys().InsertAny(L"StaticMesh_Textures_TombStone", Textures);
	}

	RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_1");
};

void StartScene::Event() & 
{
	Super::Event();

	ImGui::Begin("TTEST");
	if (ImGui::Button("ChangeScene"))
	{
		RefManager().ChangeScene<StartScene>();
	}
	if (ImGui::Button("KILL"))
	{
		for (auto& [Type,Objects]: RefManager().FindLayer<EnemyLayer>()->RefObjects())
		{
			for (auto& Obj : Objects)
			{
				auto Name = Obj->GetName(); 

				if (Name.find(L"HeightMap", 0u) != std::wstring::npos)
				{
					Obj->Kill();
				}
			}
		}
	}
	ImGui::End();
}
void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto _Map = RefManager().FindObject<EnemyLayer, Engine::HeightMap>(L"HeightMap2");
	auto _Layer = RefManager().FindLayer<EnemyLayer>();

	if (_Map)
		_Map->GetName();

	auto& LeyerMap = RefManager().RefLayers();
	
	for (auto& q : RefManager().FindObjects<EnemyLayer, Engine::HeightMap>())
	{
		//std::wcout << q->GetName() << std::endl;
	}
	for (auto& q : RefManager().FindObjects<StaticLayer, Engine::DynamicCamera>())
	{
		//std::wcout << q->GetName() << std::endl;
	}

	auto Objs2 = RefManager().RefObjects<EnemyLayer>();
}





