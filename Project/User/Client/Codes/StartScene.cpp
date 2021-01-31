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
#include "ShaderManager.h"

void StartScene::Initialize(IDirect3DDevice9* const Device)&
{

    Super::Initialize(Device);
	
	{
		// 폰트 로딩
		RefFontManager().AddFont(Device, L"Font_Default", L"바탕", 15, 20, FW_HEAVY);
		RefFontManager().AddFont(Device, L"Font_Jinji", L"궁서", 15, 20, FW_THIN);
	}

	auto& ShaderSys = RefShaderSys();
	ShaderSys.EmplaceShader(
		L"Skeleton", App::ResourcePath / L"Skeleton", {}, {}, {});

	MyModel _TestModel (
		L"..\\..\\..\\Resource\\Mesh\\DynamicMesh\\Chaos\\",
		L"Chaos.fbx", Device);


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
		// 오브젝트 스폰
		RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_1" ,
			Vector3{ 0,0,10 }, Vector3{0,0,0});

		RefManager().NewObject<EnemyLayer, TombStone>(L"Static", L"TombStone_2",
			Vector3{ 5,0,10 }, Vector3{0,0,0});
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

		for (auto& _HeightMap : RefManager().FindObjects<EnemyLayer, Engine::HeightMap>())
		{
			//std::wcout << q->GetName() << std::endl;
		}
		for (auto& _Camera : RefManager().FindObjects<StaticLayer, Engine::DynamicCamera>())
		{
			//std::wcout << q->GetName() << std::endl;
		}

		auto Objects = RefManager().RefObjects<EnemyLayer>();
	}
};







MyModel::MyModel(
	const std::filesystem::path& Path,
	const std::filesystem::path& Name,
	IDirect3DDevice9* const Device) :Device{ Device }
{
	Assimp::Importer AssimpImporter{};
	// 모델 생성 플래그 , 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.

	_Scene = AssimpImporter.ReadFile(
		(Path / Name).string(),
		aiProcess_MakeLeftHanded |
		aiProcess_FlipUVs |
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FindInstances |
		aiProcess_LimitBoneWeights |
		aiProcess_GenSmoothNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_SortByPType
	);
	CreateHierarchy(-1,_Scene->mRootNode, FMath::Identity());
	CreateMeshInformation(Path);
	VertexDecl = Vertex::Animation::GetVertexDecl(Device);
};

// Offset* anim* root* world
//본 업데이트는 업데이트마다 수항
//본 업데이트시 루트부터시작
//트랜스폼 원본이랑 오프셋 원본 저장
//초기에)
//업데이트시 애니메이션에 따라 보간해서 행렬 생성
//트랜스폼 원본* 애니메이션 보간행렬 곱해서 현재 트랜스폼 구함 그걸 트랜스폼으로 씀
//루트부터 이걸 해서 자식으로 전파하며 하향식으로 쭉 내려감 칠드런 없으면 재귀 종료
//자식은 함수 호출애 부모 트랜스폼 받아서         트랜스폼원본 곱하기 애님 보간 행렬 곱하기 부모 트랜스폼  해서 저장하는식으로 재귀 돌리는거임
//이렇게 업뎃 끝난 본 트랜스폼에가가 월드행렬만 곱하면 월드로 가는ㄱ임
//쉐이더에 넘길때는 원본오프셋행렬 곱하기 애니메이션 보간행렬  곱하기  원본이 아닌 부모 트랜스폼(루트스페이스로가는) 곱하기 가중치
//해서 누적해서 다 더한다음
//월드행렬 곱해서 하는거임
//본 월드에 맞게 업데이트
//메쉬마다 애니메이션

void MyModel::BoneUpdate()&
{
	auto iter = BoneTableIndexFromName.find
					(_Scene->mRootNode->mName.C_Str());
	auto& RootBone = BoneTable[iter->second];

	const uint32 AnimationIndex = 0u;
	static double t = 0.0;
	t += (1.0 / 60.0);
	
	for (const int32 ChildrenIdx : RootBone.ChildrensIndices)
	{
		BoneUpdateChildren(RootBone,BoneTable[ChildrenIdx],_Scene->mAnimations[AnimationIndex], t);
	}
};

void MyModel::BoneUpdateChildren(Bone& Parent,Bone& TargetBone , const aiAnimation* const _Animation, const double t)&
{
	auto* AnimationChannel = FindAnimationNode(_Animation, TargetBone.NodeName);
	// 해당 애니메이션에서 해당 관절은 움직임이 있음.  
	
	Matrix AnimationMatrix = FMath::Identity();

	if (AnimationChannel)
	{
		while (AnimationChannel->mScalingKeys[TargetBone.CurrentAnimTrack.ScaleKey].mTime < t
			&& TargetBone.CurrentAnimTrack.ScaleKey < (AnimationChannel->mNumScalingKeys - 1))
		{
			TargetBone.CurrentAnimTrack.ScaleKey = std::clamp(TargetBone.CurrentAnimTrack.ScaleKey + 1u, 0u, AnimationChannel->mNumScalingKeys - 1);
		};

		while (AnimationChannel->mRotationKeys[TargetBone.CurrentAnimTrack.RotationKey].mTime < t
			&& TargetBone.CurrentAnimTrack.RotationKey < (AnimationChannel->mNumRotationKeys - 1))
		{
			TargetBone.CurrentAnimTrack.RotationKey = std::clamp(TargetBone.CurrentAnimTrack.RotationKey + 1u, 0u, AnimationChannel->mNumRotationKeys - 1);
		};

		while (AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey].mTime < t
			&& TargetBone.CurrentAnimTrack.PositionKey < (AnimationChannel->mNumPositionKeys - 1))
		{
			TargetBone.CurrentAnimTrack.PositionKey = std::clamp(TargetBone.CurrentAnimTrack.PositionKey + 1u, 0u, AnimationChannel->mNumPositionKeys - 1);
		};
		
		const aiVectorKey ScaleBegin = AnimationChannel->mScalingKeys[TargetBone.CurrentAnimTrack.ScaleKey - 1u];
		const aiQuatKey RotationBegin = AnimationChannel->mRotationKeys[TargetBone.CurrentAnimTrack.RotationKey - 1u];
		const aiVectorKey LocationBegin=	AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey - 1u];

		const aiVectorKey ScaleEnd= AnimationChannel->mScalingKeys[TargetBone.CurrentAnimTrack.ScaleKey];
		const aiQuatKey RotationEnd = AnimationChannel->mRotationKeys[TargetBone.CurrentAnimTrack.RotationKey];
		const aiVectorKey LocationEnd = AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey];

		const float ScaleFactor    = (t - ScaleBegin.mTime )   / (ScaleEnd.mTime    - ScaleBegin.mTime);
		const float RotationFactor = (t - RotationBegin.mTime) / (RotationEnd.mTime - RotationBegin.mTime);
		const float LocationFactor = (t - LocationBegin.mTime) / (LocationEnd.mTime - LocationBegin.mTime);

		const Vector3 ScaleLhs =       FromAssimp(ScaleBegin.mValue);
		const Quaternion RotationLhs = FromAssimp(RotationBegin.mValue);
		const Vector3 LocationLhs =    FromAssimp(LocationBegin.mValue);

		const Vector3 ScaleRhs = FromAssimp(ScaleEnd.mValue);
		const Quaternion RotationRhs = FromAssimp(RotationEnd.mValue);
		const Vector3 LocationRhs = FromAssimp(LocationEnd.mValue);

		const Vector3    CurrentAnimScale    = FMath::Lerp(ScaleLhs, ScaleRhs, ScaleFactor);
		const Quaternion CurrentAnimRotation = FMath::SLerp(RotationLhs, RotationRhs, RotationFactor);
		const Vector3    CurrentAnimLocation = FMath::Lerp(LocationLhs, LocationRhs, LocationFactor);

		AnimationMatrix = FMath::Scale(CurrentAnimScale) * FMath::Rotation(CurrentAnimRotation) * FMath::Translation(CurrentAnimLocation);
	};
	
	TargetBone.CurrentTransform = TargetBone.OriginTransform * AnimationMatrix;
	TargetBone.ToRootSpace = Parent.ToRootSpace * Parent.CurrentTransform;
	TargetBone.FinalMatrix = TargetBone.Offset * AnimationMatrix * TargetBone.ToRootSpace;

	for (const int32 ChildrenIdx : TargetBone.ChildrensIndices)
	{
		BoneUpdateChildren(TargetBone, BoneTable[ChildrenIdx] ,_Animation,t);
	}
};


void MyModel::Render()&
{
	Device->SetVertexShader(nullptr);
	Device->SetPixelShader(nullptr);
	Device->SetVertexDeclaration(VertexDecl);
	for (auto& CurrentMesh : Meshes)
	{
		for (uint32 FinalTransformIdx=0u;FinalTransformIdx<CurrentMesh.FinalTransform.size();++FinalTransformIdx)
		{
			CurrentMesh.FinalTransform[FinalTransformIdx] = BoneTable[CurrentMesh.BoneTableIdxFromFinalTransformIdx[FinalTransformIdx]].FinalMatrix;
		}

		Device->SetStreamSource(0, CurrentMesh.VertexBuffer, 0, sizeof(decltype(CurrentMesh.Vertices)::value_type));
		// 텍스쳐 바인딩.
		Device->SetIndices(CurrentMesh.IndexBuffer);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0u, CurrentMesh.NumVertices, 0u, CurrentMesh.FaceCount);
	}
};

const aiNodeAnim* const MyModel::FindAnimationNode(const aiAnimation* _Animation, const std::string NodeName)&
{
	const uint32 NumChannels = _Animation->mNumChannels ;

	for (uint32 ChannelIdx = 0u; 
		ChannelIdx < NumChannels; ++ChannelIdx)
	{
		const std::string CurrentChannelNodeName = 
			_Animation->mChannels[ChannelIdx]->mNodeName.C_Str();

		if (CurrentChannelNodeName == NodeName ) 
		{
			return _Animation->mChannels[ChannelIdx];
		}
	}
	return nullptr;
}

int32 MyModel::CreateHierarchy(const uint32 ParentIdx,
	aiNode* const _Node,const Matrix ToRootSpace)&
{
	Bone _Bone;
	_Bone.OriginTransform = FromAssimp(_Node->mTransformation);
	_Bone.ToRootSpace = ToRootSpace;
	_Bone.TableIdx = BoneTable.size();
	_Bone.ParentIdx = ParentIdx;
	_Bone.NodeName = _Node->mName.C_Str();

	BoneTableIndexFromName.insert({_Node->mName.C_Str(),_Bone.TableIdx});
	BoneTable.push_back(_Bone);

	const Matrix ChildrenToRootSpace = _Bone.OriginTransform * ToRootSpace;
	for (uint32 ChildrenIdx = 0u;ChildrenIdx < _Node->mNumChildren;++ChildrenIdx)
	{
		_Bone.ChildrensIndices.push_back(
			CreateHierarchy(_Bone.ParentIdx, _Node->mChildren[ChildrenIdx], ChildrenToRootSpace));
	}
};

void MyModel::CreateMeshInformation(const std::filesystem::path& Path)&
{
	auto& ResourceSys = RefResourceSys();
	//      메쉬 인덱스 와 메쉬의 버텍스들.
	const uint32 NumMesh = _Scene->mNumMeshes;
	Meshes.resize(NumMesh);
	for (uint32 MeshIdx = 0u; MeshIdx < NumMesh; ++MeshIdx)
	{
		const aiMesh* const Current_aiMesh = _Scene->mMeshes[MeshIdx];
		auto& CurrentMesh = Meshes[MeshIdx];
		CurrentMesh.MaterialIndex = Current_aiMesh->mMaterialIndex;
		auto& CurrentMeshVertices = CurrentMesh.Vertices;

		const uint32 CurrentNumVertices = Current_aiMesh->mNumVertices;
		CurrentMeshVertices.resize(CurrentNumVertices);
		for (uint32 VerticesIdx = 0u; VerticesIdx < CurrentNumVertices; ++VerticesIdx)
		{
			Vertex::Animation TargetVertex;
			TargetVertex.Location = FromAssimp(Current_aiMesh->mVertices[VerticesIdx]);
			TargetVertex.Normal = FMath::Normalize(FromAssimp(Current_aiMesh->mNormals[VerticesIdx]));
			TargetVertex.UV = Vector2{ FromAssimp(Current_aiMesh->mTextureCoords[0][VerticesIdx]) };

			CurrentMeshVertices[VerticesIdx] = TargetVertex;
		}

		const uint32 NumFaces = Current_aiMesh->mNumFaces;
		CurrentMesh.FaceCount=NumFaces;
		for (uint32 FaceIdx = 0u; FaceIdx < NumFaces; ++FaceIdx)
		{
			aiFace CurrentFace = Current_aiMesh->mFaces[FaceIdx];
			for (uint32 IndicesIndex = 0u; IndicesIndex < CurrentFace.mNumIndices;
				++IndicesIndex)
			{
				CurrentMesh.Indices.push_back(
					CurrentFace.mIndices[IndicesIndex]
				);
			}
		}

		if (Current_aiMesh->mMaterialIndex >= 0)
		{
			aiMaterial* Material = _Scene->mMaterials[Current_aiMesh->mMaterialIndex];
			if (Material->GetTextureCount(aiTextureType_DIFFUSE) > 0u)
			{
				aiString aiTextureName;
				if (Material->GetTexture(aiTextureType_DIFFUSE, 0u, &aiTextureName
					, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
				{
					const std::string TextureName = aiTextureName.C_Str();
					std::wstring WTextureName;
					WTextureName.assign(std::begin(TextureName), std::end(TextureName));
					std::filesystem::path TexturePath = Path / TextureName;

					CurrentMesh.DiffuseMap = ResourceSys.Get<IDirect3DTexture9>(WTextureName);

					if (CurrentMesh.DiffuseMap == nullptr)
					{

						CurrentMesh.DiffuseMap =
							ResourceSys.Emplace<IDirect3DTexture9>(
								WTextureName, D3DXCreateTextureFromFile, Device,
								TexturePath.wstring().c_str(), &CurrentMesh.DiffuseMap);

					}
				}
			}
		}

		const uint32 NumBone = Current_aiMesh->mNumBones;
		CurrentMesh.FinalTransform.resize(NumBone);
		for (uint32 BoneIdx = 0u; BoneIdx < NumBone; ++BoneIdx)
		{
			const aiBone* const CurrentBone = Current_aiMesh->mBones[BoneIdx];

			CurrentBone->mOffsetMatrix;
			const uint32 CurrentBoneTableIdx = BoneTableIndexFromName.find(CurrentBone->mName.C_Str())->second; 
			BoneTable[CurrentBoneTableIdx].Offset = FromAssimp(CurrentBone->mOffsetMatrix);
			CurrentMesh.BoneTableIdxFromFinalTransformIdx.insert({ BoneIdx, CurrentBoneTableIdx });
			for (uint32 BoneAffectedVertexIdx = 0u; BoneAffectedVertexIdx < CurrentBone->mNumWeights; ++BoneAffectedVertexIdx)
			{
				const aiVertexWeight VertexWeight = CurrentBone->mWeights[BoneAffectedVertexIdx];
				const uint32 AffectedVertexIdx = VertexWeight.mVertexId;
				const float Weight = VertexWeight.mWeight;

				auto& TargetVertex = CurrentMeshVertices[AffectedVertexIdx];
				float* BoneIDsPtr = TargetVertex.BoneIds;
				float* BoneWeightsPtr = TargetVertex.BoneWeights;
				uint32 EmptyIdx = 0u;
				for (; EmptyIdx < 4u; ++EmptyIdx)
				{
					if (FMath::AlmostEqual(BoneWeightsPtr[EmptyIdx], 0.0f))
						break;
				}
				BoneWeightsPtr[EmptyIdx] = Weight;
				BoneIDsPtr[EmptyIdx] = BoneIdx;
			}
		}


	}
};

void MyModel::CreateBuffer(const std::wstring& Name)&
{
	auto& ResourceSys = RefResourceSys();

	uint32 MeshIdx = 0u;

	for (auto& CurrentMesh : Meshes)
	{
		const uint32 VertexBufferByteSize = CurrentMesh.Vertices.size() * sizeof(decltype(CurrentMesh.Vertices)::value_type);

		Device->CreateVertexBuffer(VertexBufferByteSize,
			D3DUSAGE_DYNAMIC, NULL, D3DPOOL_DEFAULT, &CurrentMesh.VertexBuffer, nullptr);

		const uint32 IndexBufferByteSize = CurrentMesh.Indices.size() * sizeof(decltype(CurrentMesh.Indices)::value_type); 

		Device->CreateIndexBuffer(IndexBufferByteSize,
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX32,D3DPOOL_MANAGED, &CurrentMesh.IndexBuffer, nullptr);

		void* VertexBufferPtr{ nullptr };
		CurrentMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr, CurrentMesh.Vertices.data(), VertexBufferByteSize);
		CurrentMesh.VertexBuffer->Unlock();

		void* IndexBufferPtr{ nullptr };
		CurrentMesh.IndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&IndexBufferPtr), NULL);
		std::memcpy(IndexBufferPtr, CurrentMesh.Indices.data(), IndexBufferByteSize);
		CurrentMesh.IndexBuffer->Unlock();

		CurrentMesh.NumVertices = CurrentMesh.Vertices.size();
		CurrentMesh.Vertices.clear();
		CurrentMesh.Indices.clear();

		ResourceSys.Insert<IDirect3DVertexBuffer9>(L"VertexBuffer_" + Name + L"_" + std::to_wstring(MeshIdx), CurrentMesh.VertexBuffer);
		ResourceSys.Insert<IDirect3DIndexBuffer9>(L"IndexBuffer_" + Name+ L"_" + std::to_wstring(MeshIdx), CurrentMesh.IndexBuffer);

		++MeshIdx;
	}
};
