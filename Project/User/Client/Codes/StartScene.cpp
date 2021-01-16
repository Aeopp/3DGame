#include "..\\stdafx.h"
#include "StartScene.h"
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
	
	auto* _Control = &RefControl();

	   D3DLOCKED_RECT LockRect; 
	   IDirect3DTexture9* ResourcePtr{ nullptr };

	   //template<typename VertexType>
	   //void CreateVertex(
		  // IDirect3DDevice9* const Device,
		  // const std::vector<VertexType>&VertexArray,
		  // const CreateVertexFlag _CreateVertexFlag,
		  // uint32 & VertexCount/*Out*/,
		  // uint32 & TriangleCount/*Out*/,
		  // uint16 & VertexByteSize/*Out*/,
		  // std::shared_ptr<IDirect3DVertexBuffer9>&VertexBuffer,/*Out*/
		  // std::shared_ptr<IDirect3DVertexDeclaration9>&VertexDecl/*Out*/
	     
	   RefResourceSys().Create<IDirect3DVertexDeclaration9>(
		   L"QQ", Vertex::Texture::GetVertexDecl(Device) );

	   // Vertex::Location3DUVTangent::GetVertexDecl 

	   /*std::vector<Vertex::Location3DUVTangent> Locationss;
	   uint32 c;
	   uint32 tc;
	   uint16 vb;
	   std::shared_ptr<IDirect3DVertexBuffer9> buf;
	   std::shared_ptr< IDirect3DVertexDeclaration9> decl;
	   RefResourceSys().Create<IDirect3DVertexBuffer9>(
		   L"QQ", DX::CreateVertex<Vertex::Location3DUVTangent>,
		   Device, Locationss, DX::CreateVertexFlag::WriteOnly,
		   c, tc, vb, buf, decl);*/

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

		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap");
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap2");
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap3");
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap4");
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap5");
		bInit = true;
	}
	else
	{
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap");
		RefManager().NewObject<EnemyLayer, Engine::HeightMap>(L"Static", L"HeightMap2");
	}


	RefManager().NewObject<StaticLayer, Engine::DynamicCamera>(
		L"Static", L"Camera",
		FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, _Control);
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
		for (auto& [Type,Objects]: RefManager().FindLayer<EnemyLayer>()->RefObjects								())
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
		std::wcout << q->GetName() << std::endl;
	}
	for (auto& q : RefManager().FindObjects<StaticLayer, Engine::DynamicCamera>())
	{
		std::wcout << q->GetName() << std::endl;
	}

	auto Objs2 = RefManager().RefObjects<EnemyLayer>();
}





