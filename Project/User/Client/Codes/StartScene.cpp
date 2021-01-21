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
#include <array>
#include "FontManager.h"

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
    Super::Initialize(Device);
	
	{
		// 폰트 로딩
		RefFontManager().AddFont(Device, L"Font_Default", L"바탕", 15, 20, FW_HEAVY);
		RefFontManager().AddFont(Device, L"Font_Jinji", L"궁서", 15, 20, FW_THIN);
	}
	

	auto* _Control = &RefControl();

	{
		// 텍스쳐 리소스 추가. 
		D3DLOCKED_RECT LockRect;
		IDirect3DTexture9* ResourcePtr{ nullptr };

		RefResourceSys().Insert<IDirect3DVertexDeclaration9>(
			L"QQ", Vertex::Texture::GetVertexDecl(Device));

		auto Tex = RefResourceSys().Emplace<IDirect3DTexture9>(
			L"Texture", D3DXCreateTextureFromFile, Device,
			(App::ResourcePath / L"Texture" / L"Player0.jpg").c_str(), &ResourcePtr);
	}

	{
		// 현재 씬 레이어 추가.
		RefManager().NewLayer<EnemyLayer>();
		RefManager().NewLayer<StaticLayer>();
	}

	{
		// 오브젝트 추가.
		static bool bInit = false;

		if (bInit == false)
		{
			{
				// 프로토타입 로딩.
				RefProto().LoadPrototype<Engine::HeightMap>(L"Static",
					Device, Engine::RenderInterface::Group::Enviroment);
				RefProto().LoadPrototype<Engine::DynamicCamera>(L"Static", Device, App::Hwnd);
				RefProto().LoadPrototype<TombStone>(L"Static", Device ,
					Engine::RenderInterface::Group::NoAlpha);
			}
			

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
	}
	

	{
		// 카메라 오브젝트 추가.
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		RefManager().NewObject<StaticLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 1000.f, Aspect, 10.f, _Control);
	}

	// 충돌체용 텍스쳐 만들기.
	{
		IDirect3DTexture9* TextureNoCollision{ nullptr };
		IDirect3DTexture9* TextureCollision{ nullptr };

		{
			Device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &TextureNoCollision, NULL);
			D3DLOCKED_RECT		LockRect;
			ZeroMemory(&LockRect, sizeof(D3DLOCKED_RECT));
			TextureNoCollision->LockRect(0, &LockRect, NULL, 0);
			*((uint32*)LockRect.pBits) = D3DXCOLOR(1.f, 0.f, 0.f, 1.f);
			TextureNoCollision->UnlockRect(0);
			RefResourceSys().Insert<IDirect3DTexture9>(L"Texture_NoCollision", TextureNoCollision);
		}

		{
			Device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &TextureCollision, NULL);
			D3DLOCKED_RECT		LockRect;
			ZeroMemory(&LockRect, sizeof(D3DLOCKED_RECT));
			TextureCollision->LockRect(0, &LockRect, NULL, 0);
			*((uint32*)LockRect.pBits) = D3DXCOLOR(0.f, 1.f, 0.f, 1.f);
			TextureCollision->UnlockRect(0);
			RefResourceSys().Insert<IDirect3DTexture9>(L"Texture_Collision",TextureCollision);
		}
	}

	{
		// 큐브용 인덱스 버퍼 로딩.

		IDirect3DIndexBuffer9* IdxBuffer{ nullptr };
		Device->CreateIndexBuffer(sizeof(Index::_16) * 12u,
			0,
			D3DFMT_INDEX16, 
			D3DPOOL_MANAGED,
			&IdxBuffer, nullptr); 
		RefResourceSys().Insert<IDirect3DIndexBuffer9>
			(L"IndexBuffer_Cube", IdxBuffer);

		Index::_16* IndexBufferPtr{ nullptr };
		IdxBuffer->Lock(0, 0, (void**)&IndexBufferPtr, 0);

		IndexBufferPtr[0]._1 = 1;
		IndexBufferPtr[0]._2 = 5;
		IndexBufferPtr[0]._3 = 6;

		IndexBufferPtr[1]._1 = 1;
		IndexBufferPtr[1]._2 = 6;
		IndexBufferPtr[1]._3 = 2;

		// x-
		IndexBufferPtr[2]._1 = 4;
		IndexBufferPtr[2]._2 = 0;
		IndexBufferPtr[2]._3 = 3;

		IndexBufferPtr[3]._1 = 4;
		IndexBufferPtr[3]._2 = 3;
		IndexBufferPtr[3]._3 = 7;

		// y+
		IndexBufferPtr[4]._1 = 4;
		IndexBufferPtr[4]._2 = 5;
		IndexBufferPtr[4]._3 = 1;

		IndexBufferPtr[5]._1 = 4;
		IndexBufferPtr[5]._2 = 1;
		IndexBufferPtr[5]._3 = 0;

		// y-
		IndexBufferPtr[6]._1 = 3;
		IndexBufferPtr[6]._2 = 2;
		IndexBufferPtr[6]._3 = 6;

		IndexBufferPtr[7]._1 = 3;
		IndexBufferPtr[7]._2 = 6;
		IndexBufferPtr[7]._3 = 7;

		// z+
		IndexBufferPtr[8]._1 = 7;
		IndexBufferPtr[8]._2 = 6;
		IndexBufferPtr[8]._3 = 5;

		IndexBufferPtr[9]._1 = 7;
		IndexBufferPtr[9]._2 = 5;
		IndexBufferPtr[9]._3 = 4;

		// z-
		IndexBufferPtr[10]._1 = 0;
		IndexBufferPtr[10]._2 = 1;
		IndexBufferPtr[10]._3 = 2;

		IndexBufferPtr[11]._1 = 0;
		IndexBufferPtr[11]._2 = 2;
		IndexBufferPtr[11]._3 = 3;

		IdxBuffer->Unlock();
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

	{
	for (size_t i = 0; i < 1000; ++i)
	{
		RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_1" + std::to_wstring(i),
			Vector3{ 0,0,10 });

		RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_2" + std::to_wstring(i),
			Vector3{ 5,0,10 });
	}
	

		/*for (size_t i = 0; i < 100; ++i)
		{
			RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_"
				+std::to_wstring(i),
				FMath::Random(Vector3{ -5,-5,-5 }, Vector3{ 5,5,5 }));
		}*/
	}
};

void StartScene::Event() & 
{
	Super::Event();

	ImGui::Begin("TTEST");
	{
		if (ImGui::Button("ChangeScene"))
		{
			// 씬전환 테스트.
			RefManager().ChangeScene<StartScene>();
		}

		if (ImGui::Button("KILL"))
		{
			// 오브젝트 삭제 테스트.
			for (auto& [Type, Objects] : 
				RefManager().FindLayer<EnemyLayer>()->RefObjects())
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
	}
	ImGui::End();
}
void StartScene::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	{
		// 오브젝트,레이어 검색 테스트.
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
}





