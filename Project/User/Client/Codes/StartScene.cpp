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
#include "UtilityGlobal.h"


void PrintLog(aiNode* _Node)
{
	if (!_Node)return;
	std::cout << _Node->mName.C_Str() << std::endl;
	FMath::DebugPrintMatrix(AssimpTo(_Node->mTransformation));
	/*std::cout << " X : " << _Node->mTransformation.a4;
	std::cout << " Y : " << _Node->mTransformation.b4;
	std::cout << " Z : " << _Node->mTransformation.c4;*/
	std::cout << std::endl;
	for (int i = 0; i < _Node->mNumChildren; ++i)
	{
		PrintLog(_Node->mChildren[i]);
	}
}





void StartScene::Initialize(IDirect3DDevice9* const Device)&
{
	/*MyModel(
		L"..\\..\\..\\Resource\\Mesh\\DynamicMesh\\Chaos\\",
		L"Chaos.fbx");*/

	Assimp::Importer AssimpImporter{};

	// 모델 생성 플래그 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.
	auto ModelScene = AssimpImporter.ReadFile( 
		"..\\..\\..\\Resource\\Mesh\\DynamicMesh\\PlayerXfile\\Player.X",
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_CalcTangentSpace |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FindInstances |
		aiProcess_LimitBoneWeights |
		aiProcess_OptimizeMeshes |
		aiProcess_GenSmoothNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_SortByPType
	);

	FMath::DebugPrintMatrix(FMath::WorldMatrix({1,1,1},
		{
			FMath::ToRadian(90.327f),
			FMath::ToRadian(18.315f),
			FMath::ToRadian(179.134f),
		}, {-294.7f,-18.645f,99.751}));
	PrintLog(ModelScene->mRootNode);

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

		RefResourceSys().Insert<IDirect3DVertexDeclaration9>(L"QQ", Vertex::Texture::GetVertexDecl(Device));

		auto Tex = RefResourceSys().Emplace<IDirect3DTexture9>(L"Texture", D3DXCreateTextureFromFile, Device,
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
	

	{
			// 오브젝트 스폰
		//RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_1" ,
		//	Vector3{ 0,0,10 }, Vector3{0,0,0});

		RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_2",
			Vector3{ 5,0,10 }, Vector3{0,0,0});
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

	if (Engine::Global::bDebugMode)
	{
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
 // "..\\..\\..\\Resource\\Mesh\\DynamicMesh\\Chaos\\Chaos.fbx" 
MyModel::MyModel(const std::filesystem::path& Path, const std::filesystem::path& Name)
{
	Assimp::Importer AssimpImporter{};
	// 모델 생성 플래그 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.
	_Scene = AssimpImporter.ReadFile(
		(Path/Name).string(),
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_CalcTangentSpace |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FindInstances |
		aiProcess_LimitBoneWeights |
		aiProcess_OptimizeMeshes |
		aiProcess_GenSmoothNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_SortByPType
	);
	CreateTextures();
	/*for (uint32 MeshIdx = 0u; MeshIdx < _Scene->mNumMeshes; ++MeshIdx)
	{
		aiMesh* _Mesh=_Scene->mMeshes[MeshIdx];
		_Mesh->bone
	}*/
	CreateHierarchy(_Scene->mRootNode);
}

void MyModel::CreateHierarchy(aiNode* const _Bone)&
{
	if (nullptr == _Bone)return;

	BoneTableFromIdx.push_back(_Bone);
	BoneTableFromName[_Bone->mName.C_Str()] = _Bone;
	BoneCount++;

	for (uint32 ChildrenIdx = 0u; ChildrenIdx < _Bone->mNumChildren; ++ChildrenIdx)
	{
		CreateHierarchy(_Bone->mChildren[ChildrenIdx]);
	}	
}

void MyModel::CreateTextures()&
{
	auto pp = _Scene->mTextures[0]->mFilename;

	for (uint32 i = 0u; i < _Scene->mNumTextures; ++i)
	{
	
	int d = 0;
	}
}
