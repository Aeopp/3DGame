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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
	Assimp::Importer AssimpImporter{};

	// 모델 생성 플래그 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.
	auto ModelScene = AssimpImporter.ReadFile( 
		"..\\..\\..\\Resource\\Mesh\\DynamicMesh\\Chaos\\Chaos.fbx",
		aiProcess_Triangulate              |
		aiProcess_JoinIdenticalVertices    |
		aiProcess_ConvertToLeftHanded      |
		aiProcess_CalcTangentSpace         |
		aiProcess_ValidateDataStructure    |
		aiProcess_ImproveCacheLocality     |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_GenUVCoords 			   |
		aiProcess_TransformUVCoords 	   |
		aiProcess_FindInstances 		   |
		aiProcess_LimitBoneWeights 		   |
		aiProcess_OptimizeMeshes 		   |
		aiProcess_GenSmoothNormals 		   |
		aiProcess_SplitLargeMeshes 		   |
		aiProcess_SortByPType			   
	);
	FMath::DebugPrintMatrix(FMath::WorldMatrix({1,1,1},
		{
			FMath::ToRadian(90.327f),
			FMath::ToRadian(18.315f),
			FMath::ToRadian(179.134f),
		}, {-294.7f,-18.645f,99.751}));
	PrintLog(ModelScene->mRootNode);

	/*std::cout << ModelScene->mRootNode->mName.C_Str() << std::endl;
	auto _RootTransform = ModelScene->mRootNode->mTransformation;
	std::cout << " X : " << _RootTransform.a4 << " Y : " << _RootTransform.b4 <<
		" Z : " << _RootTransform.c4 << std::endl;
	for (int i = 0; i < ModelScene->mRootNode->mNumChildren; ++i)
	{
		std::cout <<ModelScene->mRootNode->mChildren[i]->mName.C_Str() << std::endl;
		auto _ChildrenTransform = ModelScene->mRootNode->mChildren[i]->mTransformation;
		
		std::cout << " X : " << _ChildrenTransform.a4 << " Y : " << _ChildrenTransform.b4 <<
			" Z : " << _ChildrenTransform.c4 << std::endl;

		for (int j = 0; j < ModelScene->mRootNode->mChildren[i]->mNumChildren; ++j)
		{
			std::cout << ModelScene->mRootNode->mChildren[i]->mChildren[j]->mName.C_Str() << std::endl;
			auto _TT = ModelScene->mRootNode->mChildren[i]->mChildren[j]->mTransformation;
			std::cout << " X : " << _TT.a4 << " Y : " << _TT.b4 <<
				" Z : " << _TT.c4 << std::endl;

			int i = 0;

		}
	}*/


	// D3DXComputeBoundingBox()
	//Assimp::Importer AssimpImporter{};

	//auto ModelScene = AssimpImporter.ReadFile((App::ResourcePath / L"Mesh" / L"DynamicMesh" / L"Player" / L"Player.x"  ).string(),
	//	aiProcess_Triangulate | //사각형 정점 -> 삼각형 정점 컨버트
	//	aiProcess_JoinIdenticalVertices | // 중복 정점 하나로 합치기
	//	aiProcess_ConvertToLeftHanded | // 왼손 좌표계 변환 (노말과 탄젠트에 영향을 준다)
	//	aiProcess_GenNormals | // 모델 정보에 노말이 없을 경우 노말 생성한다. 
	//	aiProcess_CalcTangentSpace ); // 모델 정보에 탄젠트와 바이탄젠트가 없을경우 생성

	//// 메쉬 개수만큼 순회
	//for (uint32 MeshIdx = 0u; MeshIdx < ModelScene->mNumMeshes; ++MeshIdx)
	//{
	//	auto& CurrentMesh = ModelScene->mMeshes[MeshIdx];
	//	std::cout << "Mesh Name : " << CurrentMesh->mName.C_Str() << std::endl; 

	//	// 현재 메쉬 본 개수만큼 순회
	//	for (uint32 BoneIdx = 0u; BoneIdx < CurrentMesh->mNumBones; ++BoneIdx)
	//	{
	//		// 본 정보 디버깅 출력
	//		auto& CurrentBone = CurrentMesh->mBones[BoneIdx];
	//		std::cout << "Bone Name : " << CurrentBone->mName.C_Str() << std::endl;

	//		// 본의 행렬 (부모 기준 상대적인 오프셋) 
	//		CurrentBone->mOffsetMatrix;
	//		std::cout << "Bone Weights : " << CurrentBone->mWeights<< std::endl;
	//		// 본을 참조하는 버텍스를 순회
	//		for (uint32 VertexID = 0u; VertexID <CurrentBone->mNumWeights; ++VertexID)
	//		{
	//			// 버텍스 아이디와 가중치 값 디버깅 출력
	//			std::cout << "VertexID :  " << CurrentBone->mWeights[VertexID].mVertexId << std::endl;
	//			std::cout << "Vertex Weight : " << CurrentBone->mWeights[VertexID].mWeight << std::endl;
	//		};
	//	}
	//}

	//for (uint32 AnimIdx = 0u  ; AnimIdx < ModelScene->mNumAnimations; ++AnimIdx)
	//{
	//	auto& CurrentAnim = ModelScene->mAnimations[AnimIdx]; 
	//	std::cout << CurrentAnim->mName.C_Str()<< std::endl;
	//	std::cout << CurrentAnim->mTicksPerSecond << std::endl;
	//	std::cout << CurrentAnim->mDuration << std::endl;

	//	for (uint32 MeshChannelIdx = 0u; MeshChannelIdx < CurrentAnim->mNumMeshChannels; MeshChannelIdx++)
	//	{
	//		std::cout << "MeshChannelName : " << CurrentAnim->mMeshChannels[MeshChannelIdx]->mName.C_Str() << std::endl;
	//		std::cout << CurrentAnim->mMeshChannels[MeshChannelIdx]->mKeys;


	//	}
	//}

	//ModelScene->mNumMaterials; // 메쉬에 매칭되는 재질 정보 (텍스쳐 경로 , 디퓨즈 스페큘러 ,이미시브)
	//ModelScene->mMeshes[0]; // 메쉬 자료형

	//// mesh 안에 있는 정점 정보들은 인덱스로 접근 가능
	//// 대부분이 assimp 내장 벡터 형식으로 되어있음
	//// 사이즈가 같다면 reinterpret_cast 로 자신이 사용하는 자료형으로 변환해서 사용
	//// 에) XMVECTOR3 myVector = *reinterpret_cast<XMVECTOR3*>(&pScene->mMeshes[0]->mVertices);
	//ModelScene->mMeshes[0]->mVertices;
	//ModelScene->mMeshes[0]->mNormals;
	//ModelScene->mMeshes[0]->mTangents;
	//// 추가적으로 assimp는 계층구조, 애니메이션 도 있음.


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





