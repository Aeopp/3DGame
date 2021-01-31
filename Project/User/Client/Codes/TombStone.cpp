#include "..\\stdafx.h"
#include "TombStone.h"
#include "Transform.h"
#include "StaticMesh.h"

#include <iostream>
#include "StaticMesh.h"
#include "Collision.h"
#include "CollisionSystem.h"
#include "Controller.h"
#include "ExportUtility.hpp"
#include "dinput.h"
#include "imgui.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "App.h"
#include "ShaderManager.h"

static uint32 TestID = 0u;
static bool bTestCollision = false;
void TombStone::Initialize(const Vector3& SpawnLocation , const Vector3& Rotation)&
{
	Super::Initialize();


	_Model.Initialize(L"..\\..\\..\\Resource\\Mesh\\DynamicMesh\\Chaos\\",
		L"Chaos.fbx", Device);


	_TestID = TestID++;

	auto _Transform =AddComponent<Engine::Transform>();
	_Transform->SetScale({ 1,1,1 });
	_Transform->SetRotation(Rotation);
	_Transform->SetLocation(SpawnLocation);

	auto _StaticMesh =AddComponent<Engine::StaticMesh>(Device,L"TombStone");

	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::Decorator, _Transform);

	if(true)
	{
		Vector3  BoundingBoxMin{}, BoundingBoxMax{};
		D3DXComputeBoundingBox(_StaticMesh->GetVertexLocations().data(),
			_StaticMesh->GetVertexLocations().size(),
			sizeof(Vector3), &BoundingBoxMin, &BoundingBoxMax);

		_Collision->_Geometric = std::make_unique<Engine::OBB>
										(BoundingBoxMin, BoundingBoxMax);

		static_cast<Engine::OBB* const> (_Collision->_Geometric.get())->MakeDebugCollisionBox(Device);
	}
	else
	{
		Vector3 BoundingSphereCenter;
		float BoundingSphereRadius;
		D3DXComputeBoundingSphere(_StaticMesh->GetVertexLocations().data(), _StaticMesh->GetVertexLocations().size(),
			sizeof(Vector3), &BoundingSphereCenter, &BoundingSphereRadius);

		_Collision->_Geometric = std::make_unique<Engine::GSphere>(BoundingSphereRadius, BoundingSphereCenter);
		static_cast<Engine::GSphere* const>(_Collision->_Geometric.get())->MakeDebugCollisionSphere(Device);
	}

	_Collision->RefCollisionables().insert(
		{
			Engine::CollisionTag::Decorator
		});

	_Collision->RefPushCollisionables().insert(
		{
	          Engine::CollisionTag::Decorator
		});
}

void TombStone::PrototypeInitialize(IDirect3DDevice9* const Device,
						const Engine::RenderInterface::Group _Group)&
{
	Super::PrototypeInitialize(Device,_Group);
	this->Device = Device;
}

void TombStone::Event()&
{
	Super::Event();
	ImGui::Begin("CollisionTest");
	const std::string Msg = bTestCollision ? "Overlapped": "NoOverlapped" ;
	ImGui::Text(Msg.c_str());
	ImGui::End();
}

void TombStone::Render()&
{
	Super::Render();
	_Model.Render();
	const Matrix& World = GetComponent<Engine::Transform>()->UpdateWorld();
	Device->SetTransform(D3DTS_WORLD, &World);
	auto _StaticMesh = GetComponent<Engine::StaticMesh>();
	Matrix View, Proj;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Proj);
	_StaticMesh->Render();
	
	//_StaticMesh->GetMesh()->GetVertexBuffer(&VertexBuffer);
	/*IDirect3DVertexBuffer9* VertexBuffer{ nullptr }; 

	uint8* VertexBufferPtr{ nullptr }; 
	VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);

	for (uint32 Idx = 0u; Idx < _StaticMesh->GetMesh()->GetNumVertices(); ++Idx)
	{
		Vector3* const VertexLocation = 
			reinterpret_cast<Vector3*const>(  (VertexBufferPtr + (Idx * _StaticMesh->GetMesh()->GetNumBytesPerVertex()))); 
		Vector4 VectexLocationVec4 = {
		VertexLocation->x , 
		VertexLocation->y ,
		VertexLocation->z ,
		1.f };

		if (Idx == 10 && _TestID==0u )
		{
			int Debug = 0;
		}
		Matrix WorldViewProj = World * View * Proj;
		D3DXVec4Transform(&VectexLocationVec4, &VectexLocationVec4, &WorldViewProj);

		if (Idx ==10 && _TestID == 0u)
		{
			int Debug = 0;
		}
		


	}
	VertexBuffer->Unlock();*/
}

void TombStone::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	_Model.BoneUpdate();

	bTestCollision = false;

	if (_TestID == 1u )
	{
		auto& Control = RefControl();
		auto _Transform = GetComponent<Engine::Transform>();
		static constexpr float Speed = 10.f;
	/*	_Transform->Rotate({ 0,1,0 }, 3.14f * 1 * DeltaTime);
		_Transform->Rotate({ 1,0,0 }, 3.14f * 1 * DeltaTime);
		_Transform->Rotate({ 0,0,1 }, 3.14f * 1 * DeltaTime);*/

		if (Control.IsPressing(DIK_UP))
		{
			_Transform->Move({ 0,0,1 }, DeltaTime, Speed);
		}
		if (Control.IsPressing(DIK_DOWN))
		{
			_Transform->Move({ 0,0,1 },DeltaTime, -Speed);
		}
		if (Control.IsPressing(DIK_LEFT))
		{
			_Transform->Move({1,0,0} , DeltaTime, -Speed);
		}
		if (Control.IsPressing(DIK_RIGHT))
		{
			_Transform->Move({1,0,0},DeltaTime, Speed);
		}
		if (Control.IsPressing(DIK_PGUP))
		{
			_Transform->Move({ 0,1,0 },DeltaTime, Speed);
		}
		if (Control.IsPressing(DIK_PGDN))
		{
			_Transform->Move({ 0,1,0 },  DeltaTime, -Speed);
		}
	}

	if (_TestID == 0u)
	{
		auto& Control = RefControl();
		auto _Transform = GetComponent<Engine::Transform>();
		static constexpr float Speed = 10.f;
		/*	_Transform->Rotate({ 0,1,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 1,0,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 0,0,1 }, 3.14f * 1 * DeltaTime);*/

		if (Control.IsPressing(DIK_R))
		{
			_Transform->RotateYaw(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_T))
		{
			_Transform->RotateYaw(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_F))
		{
			_Transform->RotatePitch(Speed, DeltaTime); 
		}
		if (Control.IsPressing(DIK_G))
		{
			_Transform->RotatePitch(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_V))
		{
			_Transform->RotateRoll(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_B))
		{
			_Transform->RotateRoll(-Speed, DeltaTime);
		}
	}


	if (_TestID == 1u)
	{
		auto& Control = RefControl();
		auto _Transform = GetComponent<Engine::Transform>();
		static constexpr float Speed = 10.f;
		/*	_Transform->Rotate({ 0,1,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 1,0,0 }, 3.14f * 1 * DeltaTime);
			_Transform->Rotate({ 0,0,1 }, 3.14f * 1 * DeltaTime);*/

		if (Control.IsPressing(DIK_Y))
		{
			_Transform->RotateYaw(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_U))
		{
			_Transform->RotateYaw(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_H))
		{
			_Transform->RotatePitch(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_J))
		{
			_Transform->RotatePitch(-Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_N))
		{
			_Transform->RotateRoll(Speed, DeltaTime);
		}
		if (Control.IsPressing(DIK_M))
		{
			_Transform->RotateRoll(-Speed, DeltaTime);
		}
	}


}

void TombStone::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
	bTestCollision = true;
	std::cout << "충돌중" << std::endl;
};

void TombStone::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
	std::cout << "충돌Start" << std::endl;
};

void TombStone::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
	std::cout << "충돌끝!" << std::endl;

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

void MyModel::Initialize(const std::filesystem::path& Path, const std::filesystem::path& Name, IDirect3DDevice9* const Device)&
{
	this->Device = Device;

	// 모델 생성 플래그 , 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.

	_Scene = Engine::Global::AssimpImporter.ReadFile(
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

	CreateHierarchy(-1, _Scene->mRootNode, FMath::Identity());
	CreateMeshInformation(Path);
	CreateBuffer(Path / Name);
	VertexDecl = Vertex::Animation::GetVertexDecl(Device);

	auto iter = BoneTableIndexFromName.find(_Scene->mRootNode->mName.C_Str());
	auto& RootBone = BoneTable[iter->second];
	RootBone.ToRootSpace   = FMath::Identity();
	RootBone.CurrentTransform = RootBone.OriginTransform = FromAssimp(_Scene->mRootNode->mTransformation);
	RootBone.Offset = FMath::Inverse(RootBone.OriginTransform);
	RootBone.FinalMatrix = RootBone.Offset * RootBone.ToRootSpace;
}

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
		if (_Scene->HasAnimations())
			BoneUpdateChildren(RootBone, BoneTable[ChildrenIdx], _Scene->mAnimations[AnimationIndex], t);
		else
			BoneUpdateChildren(RootBone, BoneTable[ChildrenIdx], nullptr, t);
	}
};

void MyModel::BoneUpdateChildren(Bone& Parent, Bone& TargetBone, const aiAnimation* const _Animation, const double t)&
{

	// 해당 애니메이션에서 해당 관절은 움직임이 있음.  
	const aiNodeAnim* AnimationChannel{ nullptr };
	if (_Animation)
	{
		AnimationChannel = FindAnimationNode(_Animation, TargetBone.NodeName);
	}

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
		const aiVectorKey LocationBegin = AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey - 1u];

		const aiVectorKey ScaleEnd = AnimationChannel->mScalingKeys[TargetBone.CurrentAnimTrack.ScaleKey];
		const aiQuatKey RotationEnd = AnimationChannel->mRotationKeys[TargetBone.CurrentAnimTrack.RotationKey];
		const aiVectorKey LocationEnd = AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey];

		const float ScaleFactor = (t - ScaleBegin.mTime) / (ScaleEnd.mTime - ScaleBegin.mTime);
		const float RotationFactor = (t - RotationBegin.mTime) / (RotationEnd.mTime - RotationBegin.mTime);
		const float LocationFactor = (t - LocationBegin.mTime) / (LocationEnd.mTime - LocationBegin.mTime);

		const Vector3 ScaleLhs = FromAssimp(ScaleBegin.mValue);
		const Quaternion RotationLhs = FromAssimp(RotationBegin.mValue);
		const Vector3 LocationLhs = FromAssimp(LocationBegin.mValue);

		const Vector3 ScaleRhs = FromAssimp(ScaleEnd.mValue);
		const Quaternion RotationRhs = FromAssimp(RotationEnd.mValue);
		const Vector3 LocationRhs = FromAssimp(LocationEnd.mValue);

		const Vector3    CurrentAnimScale = FMath::Lerp(ScaleLhs, ScaleRhs, ScaleFactor);
		const Quaternion CurrentAnimRotation = FMath::SLerp(RotationLhs, RotationRhs, RotationFactor);
		const Vector3    CurrentAnimLocation = FMath::Lerp(LocationLhs, LocationRhs, LocationFactor);

		AnimationMatrix = FMath::Scale(CurrentAnimScale) * FMath::Rotation(CurrentAnimRotation) * FMath::Translation(CurrentAnimLocation);
	};

	if (TargetBone.TableIdx == 77)
	{
		int i = 0;
	}
	TargetBone.CurrentTransform = TargetBone.OriginTransform * AnimationMatrix;
	TargetBone.ToRootSpace = Parent.ToRootSpace * Parent.CurrentTransform;
	TargetBone.FinalMatrix = TargetBone.Offset * AnimationMatrix * TargetBone.ToRootSpace;

	for (const int32 ChildrenIdx : TargetBone.ChildrensIndices)
	{
		BoneUpdateChildren(TargetBone, BoneTable[ChildrenIdx], _Animation, t);
	}
};

void MyModel::Render()&
{
	Matrix View, Projection, ViewProjection;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	ViewProjection = View * Projection;
	auto& TargetShader = RefShaderSys().RefShader(L"Skeleton");
	TargetShader.GetSamplerIndex("DiffuseSampler");
	TargetShader.SetVSCostantData<Matrix>(Device,
		"World", FMath::Identity(), 1u);
	TargetShader.SetVSCostantData<Matrix>(Device,
		"ViewProjection", ViewProjection, 1u);
	Device->SetVertexShader(TargetShader.VsShader.get());
	Device->SetPixelShader(TargetShader.PsShader.get());
	Device->SetVertexDeclaration(VertexDecl);

	for (auto& CurrentMesh : Meshes)
	{
		for (uint32 FinalTransformIdx = 0u; FinalTransformIdx < CurrentMesh.FinalTransform.size(); ++FinalTransformIdx)
		{
			CurrentMesh.FinalTransform[FinalTransformIdx] = BoneTable[CurrentMesh.BoneTableIdxFromFinalTransformIdx[FinalTransformIdx]].FinalMatrix;
		};
		Device->SetTexture(TargetShader.GetSamplerIndex("DiffuseSampler"),
			CurrentMesh.DiffuseMap);
		TargetShader.SetVSCostantData<Matrix>(Device, "FinalMatrix",
			CurrentMesh.FinalTransform[0],
			CurrentMesh.FinalTransform.size());


		Device->SetStreamSource(0, CurrentMesh.VertexBuffer, 0, sizeof(decltype(CurrentMesh.Vertices)::value_type));
		// 텍스쳐 바인딩.
		Device->SetIndices(CurrentMesh.IndexBuffer);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0u, CurrentMesh.NumVertices, 0u, CurrentMesh.FaceCount);
	}
};

const aiNodeAnim* const MyModel::FindAnimationNode(const aiAnimation* _Animation, const std::string NodeName)&
{
	const uint32 NumChannels = _Animation->mNumChannels;

	for (uint32 ChannelIdx = 0u;
		ChannelIdx < NumChannels; ++ChannelIdx)
	{
		const std::string CurrentChannelNodeName =
			_Animation->mChannels[ChannelIdx]->mNodeName.C_Str();

		if (CurrentChannelNodeName == NodeName)
		{
			return _Animation->mChannels[ChannelIdx];
		}
	}
	return nullptr;
}

int32 MyModel::CreateHierarchy(const uint32 ParentIdx,
	aiNode* const _Node, const Matrix ToRootSpace)&
{
	Bone _Bone;
	_Bone.OriginTransform = FromAssimp(_Node->mTransformation);
	_Bone.ToRootSpace = ToRootSpace;
	_Bone.TableIdx = BoneTable.size();
	_Bone.ParentIdx = ParentIdx;
	_Bone.NodeName = _Node->mName.C_Str();
	BoneTable.push_back(_Bone);
	BoneTableIndexFromName.insert({ _Node->mName.C_Str(),_Bone.TableIdx });
	const Matrix ChildrenToRootSpace = ToRootSpace * _Bone.OriginTransform;
	for (uint32 ChildrenIdx = 0u; ChildrenIdx < _Node->mNumChildren; ++ChildrenIdx)
	{
		_Bone.ChildrensIndices.push_back(
			CreateHierarchy(_Bone.TableIdx, _Node->mChildren[ChildrenIdx], ChildrenToRootSpace));
	}
	BoneTable[_Bone.TableIdx].ChildrensIndices = _Bone.ChildrensIndices;
	return _Bone.TableIdx;
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
		CurrentMesh.FaceCount = NumFaces;
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
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &CurrentMesh.IndexBuffer, nullptr);

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
		ResourceSys.Insert<IDirect3DIndexBuffer9>(L"IndexBuffer_" + Name + L"_" + std::to_wstring(MeshIdx), CurrentMesh.IndexBuffer);

		++MeshIdx;
	}
};

