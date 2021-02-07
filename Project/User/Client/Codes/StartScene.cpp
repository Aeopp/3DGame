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
	auto& Proto = RefProto();

	

	// 텍스쳐 리소스 추가. 
	{
		IDirect3DTexture9* TexturePtr{ nullptr };
		ResourceSys.Insert<IDirect3DVertexDeclaration9>(
			L"Texture_Player", Vertex::Texture::GetVertexDecl(Device));

		auto Tex = ResourceSys.Emplace<IDirect3DTexture9>(L"Texture", D3DXCreateTextureFromFile, Device,
			(App::ResourcePath / L"Texture" / L"Player0.jpg").c_str(), &TexturePtr);
	}

	// 현재 씬 레이어 추가.
	{
		Manager.NewLayer<EnemyLayer>();
		Manager.NewLayer<StaticLayer>();
	}


	// 프로토타입 로딩.
	{
		Proto.LoadPrototype<TombStone>(L"Static", Device ,Engine::RenderInterface::Group::NoAlpha);
		Proto.LoadPrototype<Player>(L"Static", Device,Engine::RenderInterface::Group::NoAlpha);
	}

	// 카메라 오브젝트 추가.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<StaticLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 100.f, Aspect, 10.f, &Control);
	}

	{
		// 메쉬 리소스 로딩.
		ID3DXBuffer* Adjacency{ nullptr };
		ID3DXBuffer* SubSet{ nullptr };
		DWORD SubSetCount{ 0u };
		uint32 Stride{ 0u };
		uint32 VertexCount{ 0u };
		ID3DXMesh* OriginMesh{ nullptr };
		ID3DXMesh* _Mesh{ nullptr };
		auto VertexLocations =std::make_shared<std::vector<Vector3>>();
		D3DXMATERIAL* Materials;
		std::wstring ResourceName = L"TombStone";
		std::filesystem::path Path = App::ResourcePath / L"Mesh" / L"StaticMesh" / L"TombStone";
		std::filesystem::path Name = L"TombStone.x";

		RefResourceSys().Emplace<ID3DXMesh>(
			L"StaticMesh_OriginMesh_" + ResourceName
			, D3DXLoadMeshFromX, (Path / Name).c_str(),
			D3DXMESH_MANAGED,
			Device, &Adjacency, &SubSet, nullptr,
			&SubSetCount, &OriginMesh);

		uint32 FVF = OriginMesh->GetFVF();
		if (!(FVF & D3DFVF_NORMAL))
		{
			OriginMesh->CloneMeshFVF(OriginMesh->GetOptions(),
				FVF | D3DFVF_NORMAL, Device, &_Mesh);
			D3DXComputeNormals(_Mesh, reinterpret_cast<DWORD*>(Adjacency->GetBufferPointer()));
		}
		else
		{
			OriginMesh->CloneMeshFVF(OriginMesh->GetOptions(),
				FVF, Device, &_Mesh);
		}

		VertexCount = _Mesh->GetNumVertices();
		Stride = D3DXGetFVFVertexSize(FVF);

		std::array<D3DVERTEXELEMENT9, MAX_FVF_DECL_SIZE> VertexDecls;

		VertexDecls.fill(D3DVERTEXELEMENT9{});
		_Mesh->GetDeclaration(VertexDecls.data());

		uint8 _OffSet = 0u;
#pragma warning (disable :4834  )
		std::find_if(std::begin(VertexDecls), std::end(VertexDecls),
			[&_OffSet](const D3DVERTEXELEMENT9& VertexDecl)
			{
				if (VertexDecl.Usage == D3DDECLUSAGE_POSITION)
				{
					_OffSet = VertexDecl.Offset;
					return true;
				}
				else
					return false;
			});
#pragma warning (default : 4834)

		void* VertexBufferPtr{ nullptr };
		VertexLocations->resize(_Mesh->GetNumVertices());
		_Mesh->LockVertexBuffer(0, reinterpret_cast<void**>(&VertexBufferPtr));

		for (uint32 Idx = 0u; Idx < VertexCount; ++Idx)
		{
			const uint32 PtrByteJumpStride = Idx * Stride + _OffSet;

			const uint8* VertexPtrRead_1_Byte =
				reinterpret_cast<const uint8*>(VertexBufferPtr);

			VertexPtrRead_1_Byte += PtrByteJumpStride;

			Vector3 CurrentVertexLocation = 
				*reinterpret_cast<const Vector3*const>(VertexPtrRead_1_Byte);

			(*VertexLocations)[Idx] = std::move(CurrentVertexLocation);
		}
		_Mesh->UnlockVertexBuffer();

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

		RefResourceSys().InsertAny<uint32>(
			L"StaticMesh_Stride_"+ResourceName,Stride);
		RefResourceSys().InsertAny<uint32>(
			L"StaticMesh_VertexCount_"+ResourceName,VertexCount);
		RefResourceSys().InsertAny<decltype(VertexLocations)>
			(L"StaticMesh_VertexLocations_"+ ResourceName, VertexLocations);
		RefResourceSys().Insert<ID3DXMesh>(
			L"StaticMesh_Mesh_"+ ResourceName,	_Mesh);
		RefResourceSys().Insert<ID3DXBuffer>(
			L"StaticMesh_Adjacency_"+ResourceName,	Adjacency);
		RefResourceSys().Insert<ID3DXBuffer>(
			L"StaticMesh_SubSet_"+ResourceName,SubSet);
		RefResourceSys().InsertAny(
			L"StaticMesh_SubSetCount_"+ResourceName, SubSetCount);
		RefResourceSys().InsertAny(
			L"StaticMesh_Textures_"+ResourceName, Textures);
	}

	// 오브젝트 스폰
	{
		RefManager().NewObject<EnemyLayer,TombStone>(L"Static", L"TombStone_1" ,
			Vector3{ 1,1,1 },Vector3{ 0,0,10 }, Vector3{ 0,0,0 });
		RefManager().NewObject<StaticLayer,Player>(L"Static", L"Player",
			Vector3{ 0.01f,0.01f,0.01f }, Vector3{ 0,0,0}, Vector3{ 0,0,5 });

		//RefManager().NewObject<StaticLayer, Player>(L"Static", L"Player_1",
		//	Vector3{ 0.01f,0.01f,0.01f }, Vector3{ 0,0,0 }, Vector3{ 0,0,-5 });
	}
};

void StartScene::Event() & 
{
	Super::Event();

	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("SceneChange");
		{
			if (ImGui::Button("SceneChange"))
			{
				RefManager().ChangeScene<StartScene>();
			}

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


