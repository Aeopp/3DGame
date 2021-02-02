#include "SkeletonMesh.h"
#include "UtilityGlobal.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "ExportUtility.hpp"



//#include "..\\stdafx.h"
//
//
//
//// Offset* anim* root* world
////본 업데이트는 업데이트마다 수항
////본 업데이트시 루트부터시작
////트랜스폼 원본이랑 오프셋 원본 저장
////초기에)
////업데이트시 애니메이션에 따라 보간해서 행렬 생성
////트랜스폼 원본* 애니메이션 보간행렬 곱해서 현재 트랜스폼 구함 그걸 트랜스폼으로 씀
////루트부터 이걸 해서 자식으로 전파하며 하향식으로 쭉 내려감 칠드런 없으면 재귀 종료
////자식은 함수 호출애 부모 트랜스폼 받아서         트랜스폼원본 곱하기 애님 보간 행렬 곱하기 부모 트랜스폼  해서 저장하는식으로 재귀 돌리는거임
////이렇게 업뎃 끝난 본 트랜스폼에가가 월드행렬만 곱하면 월드로 가는ㄱ임
////쉐이더에 넘길때는 원본오프셋행렬 곱하기 애니메이션 보간행렬  곱하기  원본이 아닌 부모 트랜스폼(루트스페이스로가는) 곱하기 가중치
////해서 누적해서 다 더한다음
////월드행렬 곱해서 하는거임
////본 월드에 맞게 업데이트
////메쉬마다 애니메이션
//
//void MyModel::Initialize(const std::filesystem::path& Path, const std::filesystem::path& Name, IDirect3DDevice9* const Device)&
//{
//	this->Device = Device;
//
//	// 모델 생성 플래그 , 같은 플래그를 두번, 혹은 호환이 안되는
//	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
//	// 어려우므로 매우 유의 할 것.
//
//	_Scene = Engine::Global::AssimpImporter.ReadFile(
//		(Path / Name).string(),
//		aiProcess_MakeLeftHanded |
//		aiProcess_FlipUVs |
//		aiProcess_FlipWindingOrder |
//		aiProcess_Triangulate |
//		aiProcess_CalcTangentSpace |
//		aiProcess_ValidateDataStructure |
//		aiProcess_ImproveCacheLocality |
//		aiProcess_RemoveRedundantMaterials |
//		aiProcess_GenUVCoords |
//		aiProcess_TransformUVCoords |
//		aiProcess_FindInstances |
//		aiProcess_LimitBoneWeights |
//		aiProcess_GenSmoothNormals |
//		aiProcess_SplitLargeMeshes |
//		aiProcess_SortByPType
//	);
//
//	CreateHierarchy(-1, _Scene->mRootNode, FMath::Identity());
//	CreateMeshInformation(Path);
//	CreateBuffer(Path / Name);
//	VertexDecl = Vertex::Animation::GetVertexDecl(Device);
//
//	auto iter = BoneTableIndexFromName.find(_Scene->mRootNode->mName.C_Str());
//	auto& RootBone = BoneTable[iter->second];
//	RootBone.ToRootSpace = FMath::Identity();
//	RootBone.CurrentTransform = RootBone.OriginTransform = FromAssimp(_Scene->mRootNode->mTransformation);
//	RootBone.Offset = FMath::Inverse(RootBone.OriginTransform);
//	RootBone.FinalMatrix = RootBone.Offset * RootBone.ToRootSpace;
//}
//
//void MyModel::BoneUpdate()&
//{
//	auto iter = BoneTableIndexFromName.find
//	(_Scene->mRootNode->mName.C_Str());
//	auto& RootBone = BoneTable[iter->second];
//
//	const uint32 AnimationIndex = 0u;
//	static double t = 0.0;
//	t += (1.0 / 60.0);
//
//	for (const int32 ChildrenIdx : RootBone.ChildrensIndices)
//	{
//		if (_Scene->HasAnimations())
//			BoneUpdateChildren(RootBone, BoneTable[ChildrenIdx], _Scene->mAnimations[AnimationIndex], t);
//		else
//			BoneUpdateChildren(RootBone, BoneTable[ChildrenIdx], nullptr, t);
//	}
//};
//
//void MyModel::BoneUpdateChildren(Bone& Parent, Bone& TargetBone, const aiAnimation* const _Animation, const double t)&
//{
//
//	// 해당 애니메이션에서 해당 관절은 움직임이 있음.  
//	const aiNodeAnim* AnimationChannel{ nullptr };
//	if (_Animation)
//	{
//		AnimationChannel = FindAnimationNode(_Animation, TargetBone.NodeName);
//	}
//
//	Matrix AnimationMatrix = FMath::Identity();
//
//	if (AnimationChannel && false)
//	{
//		while (AnimationChannel->mScalingKeys[TargetBone.CurrentAnimTrack.ScaleKey].mTime < t
//			&& TargetBone.CurrentAnimTrack.ScaleKey < (AnimationChannel->mNumScalingKeys - 1))
//		{
//			TargetBone.CurrentAnimTrack.ScaleKey = std::clamp(TargetBone.CurrentAnimTrack.ScaleKey + 1u, 0u, AnimationChannel->mNumScalingKeys - 1);
//		};
//
//		while (AnimationChannel->mRotationKeys[TargetBone.CurrentAnimTrack.RotationKey].mTime < t
//			&& TargetBone.CurrentAnimTrack.RotationKey < (AnimationChannel->mNumRotationKeys - 1))
//		{
//			TargetBone.CurrentAnimTrack.RotationKey = std::clamp(TargetBone.CurrentAnimTrack.RotationKey + 1u, 0u, AnimationChannel->mNumRotationKeys - 1);
//		};
//
//		while (AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey].mTime < t
//			&& TargetBone.CurrentAnimTrack.PositionKey < (AnimationChannel->mNumPositionKeys - 1))
//		{
//			TargetBone.CurrentAnimTrack.PositionKey = std::clamp(TargetBone.CurrentAnimTrack.PositionKey + 1u, 0u, AnimationChannel->mNumPositionKeys - 1);
//		};
//
//		const aiVectorKey ScaleBegin = AnimationChannel->mScalingKeys[TargetBone.CurrentAnimTrack.ScaleKey - 1u];
//		const aiQuatKey RotationBegin = AnimationChannel->mRotationKeys[TargetBone.CurrentAnimTrack.RotationKey - 1u];
//		const aiVectorKey LocationBegin = AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey - 1u];
//
//		const aiVectorKey ScaleEnd = AnimationChannel->mScalingKeys[TargetBone.CurrentAnimTrack.ScaleKey];
//		const aiQuatKey RotationEnd = AnimationChannel->mRotationKeys[TargetBone.CurrentAnimTrack.RotationKey];
//		const aiVectorKey LocationEnd = AnimationChannel->mPositionKeys[TargetBone.CurrentAnimTrack.PositionKey];
//
//		const float ScaleFactor = (t - ScaleBegin.mTime) / (ScaleEnd.mTime - ScaleBegin.mTime);
//		const float RotationFactor = (t - RotationBegin.mTime) / (RotationEnd.mTime - RotationBegin.mTime);
//		const float LocationFactor = (t - LocationBegin.mTime) / (LocationEnd.mTime - LocationBegin.mTime);
//
//		const Vector3 ScaleLhs = FromAssimp(ScaleBegin.mValue);
//		const Quaternion RotationLhs = FromAssimp(RotationBegin.mValue);
//		const Vector3 LocationLhs = FromAssimp(LocationBegin.mValue);
//
//		const Vector3 ScaleRhs = FromAssimp(ScaleEnd.mValue);
//		const Quaternion RotationRhs = FromAssimp(RotationEnd.mValue);
//		const Vector3 LocationRhs = FromAssimp(LocationEnd.mValue);
//
//		const Vector3    CurrentAnimScale = FMath::Lerp(ScaleLhs, ScaleRhs, ScaleFactor);
//		const Quaternion CurrentAnimRotation = FMath::SLerp(RotationLhs, RotationRhs, RotationFactor);
//		const Vector3    CurrentAnimLocation = FMath::Lerp(LocationLhs, LocationRhs, LocationFactor);
//
//		AnimationMatrix = FMath::Scale(CurrentAnimScale) * FMath::Rotation(CurrentAnimRotation) * FMath::Translation(CurrentAnimLocation);
//	};
//
//	TargetBone.CurrentTransform = TargetBone.OriginTransform * AnimationMatrix;
//	TargetBone.ToRootSpace = Parent.ToRootSpace * Parent.CurrentTransform;
//	TargetBone.FinalMatrix = TargetBone.Offset * AnimationMatrix * TargetBone.ToRootSpace;
//
//	for (const int32 ChildrenIdx : TargetBone.ChildrensIndices)
//	{
//		BoneUpdateChildren(TargetBone, BoneTable[ChildrenIdx], _Animation, t);
//	}
//};
//
//void MyModel::Render()&
//{
//	Matrix View, Projection, ViewProjection;
//	Device->GetTransform(D3DTS_VIEW, &View);
//	Device->GetTransform(D3DTS_PROJECTION, &Projection);
//	Device->SetTransform(D3DTS_VIEW, &View);
//	Device->SetTransform(D3DTS_PROJECTION, &Projection);
//	auto Identity = FMath::Identity();
//	Device->SetTransform(D3DTS_WORLD, &Identity);
//	ViewProjection = View * Projection;
//	auto& TargetShader = RefShaderSys().RefShader(L"Skeleton");
//	TargetShader.GetSamplerIndex("DiffuseSampler");
//	TargetShader.SetVSCostantData<Matrix>(Device,
//		"World", FMath::Identity(), 1u);
//	TargetShader.SetVSCostantData<Matrix>(Device,
//		"ViewProjection", ViewProjection, 1u);
//
//	Device->SetVertexShader(nullptr);
//	Device->SetPixelShader(nullptr);
//	//Device->SetVertexShader(TargetShader.VsShader.get());
//	//Device->SetPixelShader(TargetShader.PsShader.get());
//	Device->SetVertexDeclaration(VertexDecl);
//	//Device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL |D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(2));
//	for (auto& CurrentMesh : Meshes)
//	{
//		for (uint32 FinalTransformIdx = 0u; FinalTransformIdx < CurrentMesh.FinalTransform.size();
//			++FinalTransformIdx)
//		{
//			CurrentMesh.FinalTransform[FinalTransformIdx] = BoneTable[CurrentMesh.BoneTableIdxFromFinalTransformIdx[FinalTransformIdx]].FinalMatrix;
//		};
//		Device->SetTexture(TargetShader.GetSamplerIndex("DiffuseSampler"),
//			CurrentMesh.DiffuseMap);
//		//Device->SetTexture(0,CurrentMesh.DiffuseMap); 
//		TargetShader.SetVSCostantData<Matrix>(Device, "FinalMatrix",
//			CurrentMesh.FinalTransform[0],
//			CurrentMesh.FinalTransform.size());
//
//		Device->SetStreamSource(0, CurrentMesh.VertexBuffer, 0, sizeof(decltype(CurrentMesh.Vertices)::value_type));
//		// 텍스쳐 바인딩.
//		Device->SetIndices(CurrentMesh.IndexBuffer);
//		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0u, CurrentMesh.NumVertices, 0u, CurrentMesh.FaceCount);
//	}
//};
//
//const aiNodeAnim* const MyModel::FindAnimationNode(const aiAnimation* _Animation, const std::string NodeName)&
//{
//	const uint32 NumChannels = _Animation->mNumChannels;
//
//	for (uint32 ChannelIdx = 0u;
//		ChannelIdx < NumChannels; ++ChannelIdx)
//	{
//		const std::string CurrentChannelNodeName =
//			_Animation->mChannels[ChannelIdx]->mNodeName.C_Str();
//
//		if (CurrentChannelNodeName == NodeName)
//		{
//			return _Animation->mChannels[ChannelIdx];
//		}
//	}
//	return nullptr;
//}
//
//int32 MyModel::CreateHierarchy(const uint32 ParentIdx,
//	aiNode* const _Node, const Matrix ToRootSpace)&
//{
//	Bone _Bone;
//	_Bone.OriginTransform = FromAssimp(_Node->mTransformation);
//	_Bone.ToRootSpace = ToRootSpace;
//	_Bone.TableIdx = BoneTable.size();
//	_Bone.ParentIdx = ParentIdx;
//	_Bone.NodeName = _Node->mName.C_Str();
//	BoneTable.push_back(_Bone);
//	BoneTableIndexFromName.insert({ _Node->mName.C_Str(),_Bone.TableIdx });
//	const Matrix ChildrenToRootSpace = ToRootSpace * _Bone.OriginTransform;
//	for (uint32 ChildrenIdx = 0u; ChildrenIdx < _Node->mNumChildren; ++ChildrenIdx)
//	{
//		_Bone.ChildrensIndices.push_back(
//			CreateHierarchy(_Bone.TableIdx, _Node->mChildren[ChildrenIdx], ChildrenToRootSpace));
//	}
//	BoneTable[_Bone.TableIdx].ChildrensIndices = _Bone.ChildrensIndices;
//	return _Bone.TableIdx;
//};
//
//void MyModel::CreateMeshInformation(const std::filesystem::path& Path)&
//{
//	auto& ResourceSys = RefResourceSys();
//	//      메쉬 인덱스 와 메쉬의 버텍스들.
//	const uint32 NumMesh = _Scene->mNumMeshes;
//	Meshes.resize(NumMesh);
//	for (uint32 MeshIdx = 0u; MeshIdx < NumMesh; ++MeshIdx)
//	{
//		const aiMesh* const Current_aiMesh = _Scene->mMeshes[MeshIdx];
//		auto& CurrentMesh = Meshes[MeshIdx];
//		CurrentMesh.MaterialIndex = Current_aiMesh->mMaterialIndex;
//		auto& CurrentMeshVertices = CurrentMesh.Vertices;
//
//		const uint32 CurrentNumVertices = Current_aiMesh->mNumVertices;
//		CurrentMeshVertices.resize(CurrentNumVertices);
//		for (uint32 VerticesIdx = 0u; VerticesIdx < CurrentNumVertices; ++VerticesIdx)
//		{
//			Vertex::Animation TargetVertex;
//			TargetVertex.Location = FromAssimp(Current_aiMesh->mVertices[VerticesIdx]);
//			TargetVertex.Normal = FMath::Normalize(FromAssimp(Current_aiMesh->mNormals[VerticesIdx]));
//			TargetVertex.UV = Vector2{ FromAssimp(Current_aiMesh->mTextureCoords[0][VerticesIdx]) };
//
//			CurrentMeshVertices[VerticesIdx] = TargetVertex;
//		}
//
//		const uint32 NumFaces = Current_aiMesh->mNumFaces;
//		CurrentMesh.FaceCount = NumFaces;
//		for (uint32 FaceIdx = 0u; FaceIdx < NumFaces; ++FaceIdx)
//		{
//			aiFace CurrentFace = Current_aiMesh->mFaces[FaceIdx];
//			for (uint32 IndicesIndex = 0u; IndicesIndex < CurrentFace.mNumIndices;
//				++IndicesIndex)
//			{
//				CurrentMesh.Indices.push_back(
//					CurrentFace.mIndices[IndicesIndex]
//				);
//			}
//		}
//
//		if (Current_aiMesh->mMaterialIndex >= 0)
//		{
//			aiMaterial* Material = _Scene->mMaterials[Current_aiMesh->mMaterialIndex];
//			if (Material->GetTextureCount(aiTextureType_DIFFUSE) > 0u)
//			{
//				aiString aiTextureName;
//				if (Material->GetTexture(aiTextureType_DIFFUSE, 0u, &aiTextureName
//					, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
//				{
//					const std::string TextureName = "diffuse.png";
//					aiTextureName.C_Str();
//					std::wstring WTextureName;
//					WTextureName.assign(std::begin(TextureName), std::end(TextureName));
//					std::filesystem::path TexturePath = Path / TextureName;
//
//					CurrentMesh.DiffuseMap = ResourceSys.Get<IDirect3DTexture9>(WTextureName);
//
//					if (CurrentMesh.DiffuseMap == nullptr)
//					{
//						CurrentMesh.DiffuseMap =
//							ResourceSys.Emplace<IDirect3DTexture9>(
//								WTextureName, D3DXCreateTextureFromFile, Device,
//								TexturePath.wstring().c_str(), &CurrentMesh.DiffuseMap);
//
//						CurrentMesh.DiffuseMap = TestTexture;
//
//						int i = 0;
//
//					}
//				}
//			}
//		}
//
//		const uint32 NumBone = Current_aiMesh->mNumBones;
//		CurrentMesh.FinalTransform.resize(NumBone);
//		for (uint32 BoneIdx = 0u; BoneIdx < NumBone; ++BoneIdx)
//		{
//			const aiBone* const CurrentBone = Current_aiMesh->mBones[BoneIdx];
//			const uint32 CurrentBoneTableIdx = BoneTableIndexFromName.find(CurrentBone->mName.C_Str())->second;
//			BoneTable[CurrentBoneTableIdx].Offset = FromAssimp(CurrentBone->mOffsetMatrix);
//			CurrentMesh.BoneTableIdxFromFinalTransformIdx.insert({ BoneIdx, CurrentBoneTableIdx });
//			for (uint32 BoneAffectedVertexIdx = 0u; BoneAffectedVertexIdx < CurrentBone->mNumWeights; ++BoneAffectedVertexIdx)
//			{
//				const aiVertexWeight VertexWeight = CurrentBone->mWeights[BoneAffectedVertexIdx];
//				const uint32 AffectedVertexIdx = VertexWeight.mVertexId;
//				const float Weight = VertexWeight.mWeight;
//
//				auto& TargetVertex = CurrentMeshVertices[AffectedVertexIdx];
//				float* BoneIDsPtr = TargetVertex.BoneIds;
//				float* BoneWeightsPtr = TargetVertex.BoneWeights;
//				uint32 EmptyIdx = 0u;
//				for (; EmptyIdx < 4u; ++EmptyIdx)
//				{
//					if (FMath::AlmostEqual(BoneWeightsPtr[EmptyIdx], 0.0f))
//						break;
//				}
//				BoneWeightsPtr[EmptyIdx] = Weight;
//				BoneIDsPtr[EmptyIdx] = BoneIdx;
//			}
//		}
//
//
//	}
//};
//
//void MyModel::CreateBuffer(const std::wstring& Name)&
//{
//	auto& ResourceSys = RefResourceSys();
//
//	uint32 MeshIdx = 0u;
//
//	for (auto& CurrentMesh : Meshes)
//	{
//		const uint32 VertexBufferByteSize = CurrentMesh.Vertices.size() * sizeof(decltype(CurrentMesh.Vertices)::value_type);
//
//		Device->CreateVertexBuffer(VertexBufferByteSize,
//			D3DUSAGE_DYNAMIC, NULL, D3DPOOL_DEFAULT, &CurrentMesh.VertexBuffer, nullptr);
//
//		const uint32 IndexBufferByteSize = CurrentMesh.Indices.size() * sizeof(decltype(CurrentMesh.Indices)::value_type);
//
//		Device->CreateIndexBuffer(IndexBufferByteSize,
//			D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &CurrentMesh.IndexBuffer, nullptr);
//
//		void* VertexBufferPtr{ nullptr };
//		CurrentMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
//		std::memcpy(VertexBufferPtr, CurrentMesh.Vertices.data(), VertexBufferByteSize);
//		CurrentMesh.VertexBuffer->Unlock();
//
//		void* IndexBufferPtr{ nullptr };
//		CurrentMesh.IndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&IndexBufferPtr), NULL);
//		std::memcpy(IndexBufferPtr, CurrentMesh.Indices.data(), IndexBufferByteSize);
//		CurrentMesh.IndexBuffer->Unlock();
//
//		CurrentMesh.NumVertices = CurrentMesh.Vertices.size();
//		CurrentMesh.Vertices.clear();
//		CurrentMesh.Indices.clear();
//
//		ResourceSys.Insert<IDirect3DVertexBuffer9>(L"VertexBuffer_" + Name + L"_" + std::to_wstring(MeshIdx), CurrentMesh.VertexBuffer);
//		ResourceSys.Insert<IDirect3DIndexBuffer9>(L"IndexBuffer_" + Name + L"_" + std::to_wstring(MeshIdx), CurrentMesh.IndexBuffer);
//
//		++MeshIdx;
//	}
//};



void SkeletonMesh::Load(IDirect3DDevice9* const Device)&
{
	/// <summary>
	using Type = SkeletonVertex;
	const std::wstring ResourceName = L"InputNameHere";
	const std::filesystem::path FilePath = "..\\..\\..\\Resource\\Mesh\\DynamicMesh\\Chaos\\";
	const std::filesystem::path FileName = "Chaos.fbx";
	/// </summary>
	this->Device = Device;
	// 모델 생성 플래그 , 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.
	AiScene = Engine::Global::AssimpImporter.ReadFile(
		(FilePath/FileName).string(),
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
		aiProcess_SortByPType |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph
	);

	static uint32 SkeletonResourceID = 0u;
	

	auto& ResourceSys = RefResourceSys();
	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		const std::wstring CurrentResourceName =
			ResourceName + L"_" + std::to_wstring(SkeletonResourceID++);

		Mesh CreateMesh{};
		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];
		// 버텍스 버퍼.
		std::vector<Type> Vertices{};
		for (uint32 VerticesIdx = 0u; VerticesIdx < _AiMesh->mNumVertices; ++VerticesIdx)
		{
			Vertices.push_back(Type::MakeFromAssimpMesh(_AiMesh, VerticesIdx));
		}
		
		const std::wstring MeshVtxBufResourceName =
			L"SkeletonMesh_VertexBuffer_" + CurrentResourceName;
		IDirect3DVertexBuffer9* _VertexBuffer{ nullptr };
		const uint32 VtxBufSize = sizeof(Type) * Vertices.size();
		Device->CreateVertexBuffer(VtxBufSize,D3DUSAGE_DYNAMIC, Type::FVF,
			D3DPOOL_DEFAULT, &_VertexBuffer, nullptr);
		CreateMesh.VertexBuffer =  ResourceSys.Insert<IDirect3DVertexBuffer9>(MeshVtxBufResourceName, _VertexBuffer);
		CreateMesh.PrimitiveCount=CreateMesh.FaceCount = _AiMesh->mNumFaces;
		Type* VertexBufferPtr{ nullptr };
		CreateMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr, Vertices.data(), VtxBufSize);
		CreateMesh.VertexBuffer->Unlock();
		CreateMesh.VtxCount = Vertices.size();
		// 인덱스 버퍼.
		std::vector<uint32> Indicies{};
		for (uint32 FaceIdx = 0u; FaceIdx < _AiMesh->mNumFaces; ++FaceIdx)
		{
			const aiFace CurrentFace = _AiMesh->mFaces[FaceIdx];
			for (uint32 Idx = 0u; Idx < CurrentFace.mNumIndices; ++Idx)
			{
				Indicies.push_back(CurrentFace.mIndices[Idx]);
			}
		}
		const std::wstring MeshIdxBufResourceName =
			L"SkeletonMesh_IndexBuffer_" + CurrentResourceName;
		IDirect3DIndexBuffer9* _IndexBuffer{ nullptr };
		const uint32 IdxBufSize = sizeof(uint32) * Indicies.size();
		Device->CreateIndexBuffer(IdxBufSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32,
			D3DPOOL_MANAGED, &_IndexBuffer, nullptr);
		CreateMesh.IndexBuffer = ResourceSys.Insert<IDirect3DIndexBuffer9>(MeshIdxBufResourceName,
			_IndexBuffer);
		uint32* IndexBufferPtr{ nullptr };
		CreateMesh.IndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&IndexBufferPtr), NULL);
		std::memcpy(IndexBufferPtr, Indicies.data(), IdxBufSize);
		CreateMesh.IndexBuffer->Unlock();
		// 머테리얼.
		aiMaterial* AiMaterial = AiScene->mMaterials[_AiMesh->mMaterialIndex];
		if (AiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType_DIFFUSE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_DiffuseTexture_" + CurrentResourceName;

				RefResourceSys().Emplace<IDirect3DTexture9>
					(TexResourceName,
					D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.DiffuseTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_NormalTexture_" + CurrentResourceName;

				RefResourceSys().Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.NormalTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_SpecularTexture_" + CurrentResourceName;

				RefResourceSys().Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.SpecularTexture);
			}
		}
		MeshContainer.push_back(CreateMesh);
	}


}

void SkeletonMesh::Render()&
{
	/// <summary>
	using Type = SkeletonVertex;
	/// </summary>
	Device->SetFVF(Type::FVF);
	Device->SetVertexShader(nullptr);
	Device->SetPixelShader(nullptr);
	for (auto& CurrentRenderMesh : MeshContainer)
	{
		Device->SetTexture(0, CurrentRenderMesh.DiffuseTexture);
		Device->SetStreamSource(0, CurrentRenderMesh.VertexBuffer, 0, sizeof(Type));
		Device->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,0u,0u,CurrentRenderMesh.VtxCount,0u,CurrentRenderMesh.PrimitiveCount);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0u, CurrentRenderMesh.FaceCount);
	}
}
